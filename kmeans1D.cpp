#include <iostream>
#include <cstdio>
#include <vector>
#include <cmath>
#include <chrono>
using namespace std;

float distancia(int p1, int p2, const vector<float>& points, int row, int column) {
    float suma = 0;
    for (int i = 0; i < column; i++) { 
        suma += (points[p1 * column + i] - points[p2 * column + i])*(points[p1 * column + i] - points[p2 * column + i]);
    }
    return sqrt(suma);
}

int puntoMasLejano(vector<int> clusters, const vector<float>& points, int row, int column) {
    int n_clusteres = clusters.size();
    float masLejano = 0;
    int indice = 0;
    for (int i = 0; i < row; i++) {
        float d = 0;
        for (int j = 0; j < n_clusteres; j++) {
            d += distancia(clusters[j], i, points, row, column); 
        }
        if (d > masLejano) {
            masLejano = d;
            indice = i;
        }
    }
    return indice;
}

vector<int> selectorClusters(int n_clusters, const vector<float>& points, int row, int column) {
    vector<int> clusters;
    clusters.push_back(0);
    for (int i = 0; i < n_clusters - 1; i++) {
        clusters.push_back(puntoMasLejano(clusters, points, row, column));
    }
    return clusters;
}

vector<vector<int>> puntosSeparadosPorCluster(const vector<int>& p_clusters, const vector<float>& points, int row, int column) {
    int n_clusters = p_clusters.size();
    vector<vector<int>> clusters(n_clusters);
    for (int i = 0; i < row; i++) {
        float d = -1, indice = 0;
        for (int j = 0; j < n_clusters; j++) {
            float temp_d = distancia(p_clusters[j], i, points, row, column);
            if (d == -1) d = temp_d;
            else if (d > temp_d) {
                d = temp_d;
                indice = j;
            }
        }
        clusters[indice].push_back(i);
    }
    return clusters;
}

vector<float> minimo(const vector<int>& cluster, const vector<float>& points, int row, int column) {
    vector<float> p_minimo(column);
    for (int i = 0; i < column; i++) p_minimo[i] = points[cluster[0] * 3 + i];
    int n_puntos = cluster.size();
    for (int i = 0; i < n_puntos; i++) {
        for (int j = 0; j < column; j++) {
            if (p_minimo[j] > points[cluster[i] * 3 + j]) {
                p_minimo[j] = points[cluster[i] * 3 + j];
            }
        }
    }
    return p_minimo; 
}

vector<float> maximo(const vector<int>& cluster, const vector<float>& points, int row, int column) {
    vector<float> p_maximo(column);
    for (int i = 0; i < column; i++) p_maximo[i] = points[cluster[0] * 3 + i];
    int n_puntos = cluster.size();
    for (int i = 0; i < n_puntos; i++) {
        for (int j = 0; j < column; j++) {
            if (p_maximo[j] < points[cluster[i] * 3 + j]) {
                p_maximo[j] = points[cluster[i] * 3 + j];
            }
        }
    }
    return p_maximo;
}

vector<float> media(const vector<int>& cluster, const vector<float>& points, int row, int column) {
    vector<float> p_medio(column);
    int n_puntos = cluster.size();
    for (int i = 0; i < n_puntos; i++) {
        for (int j = 0; j < column; j++) {
            p_medio[j] += points[cluster[i] * 3 + j];
        }
    }
    for (int i = 0; i < column; i++) p_medio[i] = p_medio[i] / n_puntos;
    return p_medio;
}

vector<float> varianza(const vector<int>& cluster,vector<float> p_medio,const vector<float>& points, int row, int column) {
    vector<float> p_var(column);
    int n_puntos = cluster.size();
    for (int i = 0; i < n_puntos; i++) {
        for (int j = 0; j < column; j++) {
            float temp = points[cluster[i] * 3 + j] - p_medio[j];
            p_var[j] += temp*temp;
        }
    }
    for (int i = 0; i < column; i++) p_var[i] = p_var[i] / (n_puntos - 1);
    return p_var;
}


int main() {
    FILE *archivo = nullptr;
    archivo = fopen("salida", "rb"); 
    if (archivo == NULL) return 1;
    
    vector<float> all_points;
    int row, column;

    fread(&row, sizeof(int), 1, archivo);
    fread(&column, sizeof(int), 1, archivo);
    for (int i = 0; i < row ; i++) { 
		float value;
		for (int j = 0; j < column; j++) { 
			fread(&value, sizeof(float), 1, archivo); 
			all_points.push_back(value); 
		} 
	}

	auto init_time = chrono::high_resolution_clock::now();
                                                                                                         
	vector<int> indices_p_clusters = selectorClusters(3, all_points, row, column);
	vector<vector<int>> clusters = puntosSeparadosPorCluster(indices_p_clusters, all_points, row, column);
	for (int i = 0; i < 3; i++) {
		vector<float> min = minimo(clusters[i], all_points, row, column);
		vector<float> max = maximo(clusters[i], all_points, row, column);
		vector<float> med = media(clusters[i], all_points, row, column);
		vector<float> var = varianza(clusters[i], med, all_points, row, column);
		cout << "Cluster " << i << endl;
		cout << "Minimo: " << min[0] << " " << min[1] << " " << min[2] << endl; 
		cout << "Maximo: " << max[0] << " " << max[1] << " " << max[2] << endl; 
		cout << "Media: " << med[0] << " " << med[1] << " " << med[2] << endl; 
		cout << "Varianza: " << var[0] << " " << var[1] << " " << var[2] << endl; 
	} 
	auto final_time = chrono::high_resolution_clock::now();
	auto duration = chrono::duration_cast<chrono::microseconds>(final_time - init_time).count();
	cout << "The core of the program finished in: " << duration/1000000.0 << " sec." << endl;

    fclose(archivo);
    return 0;
}
