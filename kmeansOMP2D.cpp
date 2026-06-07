#include <iostream>
#include <cstdio>
#include <vector>
#include <cmath>
#include <chrono>
#include <omp.h>
using namespace std;

float distancia(const vector<float>& p1, const vector<float>& p2, int row, int column) {
    float suma = 0;
    for (int i = 0; i < column; i++) { 
        suma += (p1[i] - p2[i]) * (p1[i] - p2[i]);
    }
    return sqrt(suma);
}

int puntoMasLejano(vector<int> clusters, const vector<vector<float>>& points, int row, int column) {
    int n_clusteres = clusters.size();
    float masLejano = 0;
    int indice = 0;
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

vector<int> selectorClusters(int n_clusters, const vector<vector<float>>& points, int row, int column) {
    vector<int> clusters;
    clusters.push_back(0);
    for (int i = 0; i < n_clusters - 1; i++) {
        clusters.push_back(puntoMasLejano(clusters, points, row, column));
    }
    return clusters;
}

vector<vector<int>> puntosSeparadosPorCluster(const vector<int>& p_clusters, const vector<vector<float>>& points, int row, int column) {
    int n_clusters = p_clusters.size();
	int n_points = points.size();
    vector<vector<int>> clusters(n_clusters);
    for (int i = 0; i < n_points; i++) {
        float d = -1;
		int indice = 0;
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

vector<float> minimo(const vector<int>& cluster, const vector<vector<float>>& points, int row, int column) {
    float p_minimo[column];
	vector<float> v_minimo(column);
    for (int i = 0; i < column; i++) p_minimo[i] = points[cluster[0]][i];
    int n_puntos = cluster.size();
	#pragma omp parallel for reduction(min:p_minimo[:column])
    for (int i = 0; i < n_puntos; i++) {
        for (int j = 0; j < column; j++) {
            if (p_minimo[j] > points[cluster[i]][j]) {
                p_minimo[j] = points[cluster[i]][j];
            }
        }
    }
	for (int i = 0; i < column; i++) v_minimo[i] = p_minimo[i];
    return v_minimo; 
}

vector<float> maximo(const vector<int>& cluster, const vector<vector<float>>& points, int row, int column) {
    float p_maximo[column];
	vector<float> v_maximo(column);
    for (int i = 0; i < column; i++) p_maximo[i] = points[cluster[0]][i];
    int n_puntos = cluster.size();
	#pragma omp parallel for reduction(max:p_maximo[:column])
    for (int i = 0; i < n_puntos; i++) {
        for (int j = 0; j < column; j++) {
            if (p_maximo[j] < points[cluster[i]][j]) {
                p_maximo[j] = points[cluster[i]][j];
            }
        }
    }	
	for (int i = 0; i < column; i++) v_maximo[i] = p_maximo[i];
    return v_maximo;
}

vector<float> media(const vector<int>& cluster, const vector<vector<float>>& points, int row, int column) {
	float p_medio[column] = {0};
	vector<float> v_medio(column);
    int n_puntos = cluster.size();
	#pragma omp parallel for reduction(+:p_medio[:column])
    for (int i = 0; i < n_puntos; i++) {
        for (int j = 0; j < column; j++) {
            p_medio[j] += points[cluster[i]][j];
        }
    }
    for (int i = 0; i < column; i++) v_medio[i] = p_medio[i] / n_puntos;
    return v_medio;
}

vector<float> varianza(const vector<int>& cluster,vector<float> p_medio,const vector<vector<float>>& points, int row, int column) {
    float p_var[column] = {0};
	vector<float> v_var(column);
    int n_puntos = cluster.size();
	#pragma omp parallel for reduction(+:p_var[:column])
    for (int i = 0; i < n_puntos; i++) {
        for (int j = 0; j < column; j++) {
            float temp = points[cluster[i]][j] - p_medio[j];
            p_var[j] += temp*temp;
        }
    }
    for (int i = 0; i < column; i++) v_var[i] = p_var[i] / (n_puntos - 1);
    return v_var;
}


int main() {
    FILE *archivo = nullptr;
    archivo = fopen("salida", "rb"); 
    if (archivo == NULL) return 1;
    
    vector<vector<float>> all_points;
    int row, column;

    fread(&row, sizeof(int), 1, archivo);
    fread(&column, sizeof(int), 1, archivo);
    for (int i = 0; i < row; i++) { vector<float> local_point; float value; for (int j = 0; j < column; j++) { fread(&value, sizeof(float), 1, archivo); local_point.push_back(value); } all_points.push_back(local_point); }

	auto init_time = chrono::high_resolution_clock::now();

	vector<int> indices_p_clusters = selectorClusters(3, all_points, row, column);
	vector<vector<int>> clusters = puntosSeparadosPorCluster(indices_p_clusters, all_points, row, column);
	#pragma omp paralell 
	{
		#pragma omp paralell for
		for (int i = 0; i < 3; i++) {
			vector<float> min = minimo(clusters[i], all_points, row, column);
			vector<float> max = maximo(clusters[i], all_points, row, column);
			vector<float> med = media(clusters[i], all_points, row, column);
			vector<float> var = varianza(clusters[i], med, all_points, row, column);
			#pragma omp critical
			{
				cout << "Cluester " << i << endl;
				cout << "Minimo: " << min[0] << " " << min[1] << " " << min[2] << endl; 
				cout << "Maximo: " << max[0] << " " << max[1] << " " << max[2] << endl; 
				cout << "Media: " << med[0] << " " << med[1] << " " << med[2] << endl; 
				cout << "Varianza: " << var[0] << " " << var[1] << " " << var[2] << endl; 
			}
		}
	}
	auto final_time = chrono::high_resolution_clock::now();
	auto duration = chrono::duration_cast<chrono::microseconds>(final_time - init_time).count();
	cout << "The core of the program finished in: " << duration/1000000.0 << " sec." << endl;

    fclose(archivo);
    return 0;
}
