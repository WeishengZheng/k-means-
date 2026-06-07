#include <iostream>
#include <cstdio>
#include <vector>
#include <cmath>
#include <chrono>
using namespace std;

vector<float> centroide(vector<vector<float>> points,int row,int column) {
    vector<float> punto_centroide;
    for (int i = 0; i < column; i++) punto_centroide.push_back(0);
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < column; j++) {
            punto_centroide[j] = punto_centroide[j] + points[i][j];
        }
    }
    for (int i = 0; i < column; i++) {
        punto_centroide[i] = punto_centroide[i] / row;
    }
    return punto_centroide;
}

float distancia(vector<float> p1, vector<float> p2, int row, int column) {
    float suma = 0;
    for (int i = 0; i < column; i++) { 
        suma += (p1[i] - p2[i]) * (p1[i] - p2[i]);
    }
    return sqrt(suma);
}

int puntoMasLejano(vector<int> clusters,vector<vector<float>> points, int row, int column) {
    int n_clusteres = clusters.size();
    int masLejano = 0;
    float indice = 0;
    for (int i = 0; i < row; i++) {
        float d = 0;
        for (int j = 0; j < n_clusteres; j++) {
            d += distancia(points[clusters[j]], points[i], row, column); 
        }
        if (d > masLejano) {
            masLejano = d;
            indice = i;
        }
    }
    return indice;
}

vector<int> selectorClusters(int n_clusters, vector<vector<float>> points, int row, int column) {
    vector<int> clusters;
    clusters.push_back(0);
    for (int i = 0; i < n_clusters - 1; i++) {
        clusters.push_back(puntoMasLejano(clusters, points, row, column));
    }
    return clusters;
}

vector<vector<int>> puntosSeparadosPorCluster(vector<int> p_clusters, vector<vector<float>> points, int row, int column) {
    int n_clusters = p_clusters.size();
	int n_points = points.size();
    vector<vector<int>> clusters(n_clusters);
    for (int i = 0; i < n_points; i++) {
        float d = -1, indice = 0;
        for (int j = 0; j < n_clusters; j++) {
            float temp_d = distancia(points[p_clusters[j]], points[i], row, column);
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

vector<float> minimo(vector<int> cluster, vector<vector<float>> points, int row, int column) {
    vector<float> p_minimo(column);
    for (int i = 0; i < column; i++) p_minimo[i] = points[cluster[0]][i];
    int n_puntos = cluster.size();
    for (int i = 0; i < n_puntos; i++) {
        for (int j = 0; j < column; j++) {
            if (p_minimo[j] > points[cluster[i]][j]) {
                p_minimo[j] = points[cluster[i]][j];
            }
        }
    }
    return p_minimo; 
}

vector<float> maximo(vector<int> cluster, vector<vector<float>> points, int row, int column) {
    vector<float> p_maximo(column);
    for (int i = 0; i < column; i++) p_maximo[i] = points[cluster[0]][i];
    int n_puntos = cluster.size();
    for (int i = 0; i < n_puntos; i++) {
        for (int j = 0; j < column; j++) {
            if (p_maximo[j] < points[cluster[i]][j]) {
                p_maximo[j] = points[cluster[i]][j];
            }
        }
    }
    return p_maximo;
}

vector<float> media(vector<int> cluster,vector<vector<float>> points, int row, int column) {
    vector<float> p_medio(column);
    int n_puntos = cluster.size();
    for (int i = 0; i < n_puntos; i++) {
        for (int j = 0; j < column; j++) {
            p_medio[j] += points[cluster[i]][j];
        }
    }
    for (int i = 0; i < column; i++) p_medio[i] = p_medio[i] / n_puntos;
    return p_medio;
}

vector<float> varianza(vector<int> cluster,vector<float> p_medio,vector<vector<float>> points, int row, int column) {
    vector<float> p_var(column);
    int n_puntos = cluster.size();
    for (int i = 0; i < n_puntos; i++) {
        for (int j = 0; j < column; j++) {
            float temp = points[cluster[i]][j] - p_medio[j];
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
    
    vector<vector<float>> all_points;
    int row, column;

    fread(&row, sizeof(int), 1, archivo);
    fread(&column, sizeof(int), 1, archivo);
    
    for (int i = 0; i < row; i++) {
        vector<float> local_point;
        float value;
        for (int j = 0; j < column; j++) {
            fread(&value, sizeof(float), 1, archivo);
            local_point.push_back(value);
        }
        all_points.push_back(local_point);
    }

	auto init_time = chrono::high_resolution_clock::now();

	vector<int> indices_p_clusters = selectorClusters(3, all_points, row, column);
	vector<vector<int>> clusters = puntosSeparadosPorCluster(indices_p_clusters, all_points, row, column);
	for (int i = 0; i < 3; i++) {
		vector<float> min = minimo(clusters[i], all_points, row, column);
		vector<float> max = maximo(clusters[i], all_points, row, column);
		vector<float> med = media(clusters[i], all_points, row, column);
		vector<float> var = varianza(clusters[i], med, all_points, row, column);
		cout << "Cluester " << i << endl;
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
