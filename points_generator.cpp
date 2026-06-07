#include <iostream>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <ctime>

#define PI 3.141592653588979f

struct point {
    std::vector<float> values;
};

point getRandomPoint(point center ,float maxradius, float minradius = 0.0f )
{
    point p;
    int dimensions = center.values.size();
    float suma_cuadrados = 0.0f;
    std::vector<float> coords(dimensions);

    for (int i = 0; i < dimensions ; i++) {
        float p1 = (float)rand() / RAND_MAX;
        float p2 = (float)rand() / RAND_MAX;
        
        if (p1 < 0.00001f) p1 = 0.00001f;

        coords[i]  = sqrt(-2.0 * log(p1)) * cos(2.0f * PI * p2);
        suma_cuadrados += coords[i] * coords[i];
    }

    float magnitud = sqrt(suma_cuadrados);
    float alpha = (float)rand() / RAND_MAX;
    float r = pow(pow(minradius, dimensions) + (pow(maxradius, dimensions) - pow(minradius, dimensions)) * alpha, 1.0f / dimensions);

    for (int i = 0; i < dimensions; i++) p.values.push_back(center.values[i] + (coords[i] / magnitud * r));

    return p;
};

int main()
{
    srand(time(NULL));
    int nClusters = 3;
    int nPointsPerCluster = 10000000;
    int dimensions = 3;

    std::vector<point> data;
    point center;
    for (int i = 0; i < dimensions; i++) center.values.push_back(0.0f);
    for (int i = 0; i < nClusters; i++)
    {
        point centroid = getRandomPoint(center, 20.0, 0.0);
        for (int j = 0; j < nPointsPerCluster; j++)
            data.push_back(getRandomPoint(centroid, 1.0f));
    }
    FILE* resultsFile = fopen("salida", "wb");
    int nFilas = nClusters * nPointsPerCluster;
    int nCol = dimensions;
    fwrite(&nFilas, sizeof(int), 1, resultsFile);
    fwrite(&nCol, sizeof(int), 1, resultsFile);
    for (int i = 0; i < nFilas; i++) {
        fwrite(data[i].values.data(), sizeof(float), nCol, resultsFile);  
    }
    fclose(resultsFile);
	/*
    for (int i = 0; i < nPointsPerCluster * nClusters; i++) {
        for (int j = 0; j < dimensions; j++) {
            if (j != dimensions - 1) std::cout << data[i].values[j] << "\t";
            else std::cout << data[i].values[j] << "\n";
        }
    }*/
}
