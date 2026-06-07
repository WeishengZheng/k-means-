#include <mpi.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
using namespace std;

float distancia(const vector<float>& p1, int p2, const vector<float>& points, int column) {	
    float suma = 0;
    for (int i = 0; i < column; i++) { 
        suma += (p1[i] - points[p2 * column + i])*(p1[i] - points[p2 * column + i]);
    }
    return suma;
}

int puntoMasLejano(const vector<vector<float>>& clusters, const vector<float>& points, int row, int column) {
    int n_clusteres = clusters.size();
    float masLejano = 0;
    int indice = 0;
    for (int i = 0; i < row; i++) {
        float d = 0;
        for (int j = 0; j < n_clusteres; j++) {
            d += distancia(clusters[j], i, points, column); 
        }
        if (d > masLejano) {
            masLejano = d;
            indice = i;
        }
    }
    return indice;
}

vector<vector<int>> puntosSeparadosPorCluster(const vector<vector<float>>& p_clusters, const vector<float>& points, int row, int column) {
    int n_clusters= p_clusters.size();
    vector<vector<int>> clusters(n_clusters);
    for (int i = 0; i < row; i++) {
        float d = -1;
		int indice = 0;
        for (int j = 0; j < n_clusters; j++) {
            float temp_d = distancia(p_clusters[j], i, points, column);
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

int main(int argc, char** argv) {
	MPI_Init(&argc, &argv);

	MPI_Status status;

	int N_CLUSTERS = 3;

	int rank, size;
	int row, column, headerOfset;

	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	int datos[3];

	if (rank == 0) {	
		FILE *archivo = nullptr;
		archivo = fopen("salida", "rb"); 
		if (archivo == NULL) return 1;
		
		vector<vector<float>> puntos_local;

		fread(&row, sizeof(int), 1, archivo);
		fread(&column, sizeof(int), 1, archivo);

		fclose(archivo);
		
		headerOfset = sizeof(int) * 2;

		datos[0] = headerOfset;
		datos[1] = column;
		datos[2] = row;
	}

	MPI_Bcast(datos, 3, MPI_INT, 0, MPI_COMM_WORLD);
	if (rank != 0) {
		row = datos[2];
		column = datos[1];
		headerOfset = datos[0];
	}

	int sobra = row%size;
	int localSize = row/size;
	int extra = 0;

	if (rank < sobra) localSize++;
	else extra = sobra;

	long long localOffset = ((long long)sizeof(float) * column * (rank * localSize + extra)) + (long long)headerOfset;

	MPI_File fh;

	int err = MPI_File_open(MPI_COMM_WORLD, "salida", MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
	
	if (err != MPI_SUCCESS) cout << "No se ha encontrado el archivo" << endl;
	
	vector<float> puntos_local(localSize * column);

	MPI_File_read_at(fh, localOffset, puntos_local.data(), localSize * column, MPI_FLOAT, &status);

	MPI_File_close(&fh);

	vector<vector<float>> p_clusters(1, vector<float>(column));
	float centroide[column];

	auto init_time = chrono::high_resolution_clock::now();

	if (rank == 0) {
		int idx = 0;
		for (int i = 0; i < column; i++) centroide[i] = puntos_local[i];		
	}

	MPI_Bcast(centroide, column, MPI_FLOAT, 0, MPI_COMM_WORLD);

	p_clusters[0].assign(centroide, centroide + column);	

	struct {
		float distancia;
		int rank;
	} local_max, global_max;

	local_max.rank = rank;

	for (int i = 0; i < N_CLUSTERS-1; i++) {
		int local_further = puntoMasLejano(p_clusters, puntos_local, localSize, column);
		local_max.distancia = 0;

		for (int j = 0; j < i + 1; j++) local_max.distancia += distancia(p_clusters[j], local_further, puntos_local, column); 

		MPI_Allreduce(&local_max, &global_max, 1, MPI_FLOAT_INT, MPI_MAXLOC, MPI_COMM_WORLD);

		if (rank == global_max.rank) {
			for (int i = 0; i < column; i++) centroide[i] = puntos_local[local_further * column + i];
		}

		MPI_Bcast(centroide, column, MPI_FLOAT, global_max.rank, MPI_COMM_WORLD);

		p_clusters.push_back(vector<float>(centroide, centroide + column));
	}

	vector<vector<int>> local_clusters = puntosSeparadosPorCluster(p_clusters, puntos_local, localSize, column);

	for (int c = 0; c < N_CLUSTERS; c++) {
		vector<float> min_local(column, INFINITY);
		vector<float> max_local(column, -INFINITY);	
    	vector<float> med_local(column, 0);
		int indice;
		
		int n_puntos = local_clusters[c].size();
		for (int i = 0; i < n_puntos; i++) {
			for (int j = 0; j < column; j++) {
				indice = local_clusters[c][i] * column + j;
				if (min_local[j] > puntos_local[indice]) {
					min_local[j] = puntos_local[indice];
				}
				if (max_local[j] < puntos_local[indice]) {
					max_local[j] = puntos_local[indice];
				}
				med_local[j] += puntos_local[indice];
			}
		}

		vector<float> min(column);
		vector<float> max(column);	
    	vector<float> med(column);

		int cluster_size[column];

		for (int i = 0; i < column; i++) {
			MPI_Allreduce(&min_local[i], &min[i], 1, MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD);
			MPI_Allreduce(&max_local[i], &max[i], 1, MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD);
			MPI_Allreduce(&med_local[i], &med[i], 1, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);
			MPI_Allreduce(&n_puntos, &cluster_size[c], 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
		}

    	for (int i = 0; i < column; i++) med[i] = med[i] / cluster_size[c];

		if (rank == 0) {
			cout << "Cluster " << c << endl;
			cout << "Minimo: " << min[0] << " " << min[1] << " " << min[2] << endl; 
			cout << "Maximo: " << max[0] << " " << max[1] << " " << max[2] << endl; 
			cout << "Media: " << med[0] << " " << med[1] << " " << med[2] << endl; 
		}

		vector<float> var_local(column);
		float temp;
		for (int i = 0; i < n_puntos; i++) {
			for (int j = 0; j < column; j++) {
				temp = puntos_local[local_clusters[c][i] * column + j] - med[j];
				var_local[j] += temp*temp;
			}
		}

		vector<float> var(column);

		for (int i = 0; i < column; i++) {
			MPI_Allreduce(&var_local[i], &var[i], 1, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);
		}

		for (int i = 0; i < column; i++) var[i] = var[i] / (cluster_size[c] - 1);
		
		if (rank == 0) {
			cout << "Varianza: " << var[0] << " " << var[1] << " " << var[2] << endl;
		}
	}

	if (rank == 0) {
		auto final_time = chrono::high_resolution_clock::now();
		auto duration = chrono::duration_cast<chrono::microseconds>(final_time - init_time).count();
		cout << "The core of the program finished in: " << duration/1000000.0 << " sec." << endl;
	}
	
	MPI_Finalize();
	return 0;
}
