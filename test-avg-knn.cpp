#include "lib/DataSet.cpp"
#include "lib/Config.cpp"
#include "./res/kmeans/kmeans.cpp"
#include "./res/hnswlib/hnswlib/hnswlib.h"


int main(int argc, char **argv ){
    if (argc != 3 && argc != 5) {
        std::cout << "Usage: ./hnsw <M> <ef_construction> \n or ./hnsw <M> <ef_construction> <CLUSTER_NUM> <CLUSTER_CORE>" << std::endl;
        return 0;
    }
    M = atoi(argv[1]);
    ef_construction = atoi(argv[2]);
    if (argc == 5) {
        CLUSTER_NUM = atoi(argv[3]);
        CLUSTER_CORE = atoi(argv[4]);
    }
    std::ios::sync_with_stdio(false);
    std::cin.tie(0);
    HDF5DataSet<FILETYPE> *ds = new HDF5DataSet<FILETYPE>(baseFileName);
    DataSet<FILETYPE> *dataSet = ds;
    std::vector<double> v2v_k100_avg_dist;
    #pragma omp parallel for
    for (auto & v : ds->baseData) {
        std::vector<double> v2v_k100_dist;
        for (auto & v2 : ds->baseData) {
            v2v_k100_dist.push_back(v2 - v);
        }
        std::sort(v2v_k100_dist.begin(), v2v_k100_dist.end());
        double avg_dist = 0;
        for (int i = 0; i < 100; i++) {
            avg_dist += v2v_k100_dist[i];
        }
        avg_dist /= 100;
        pfbugs(avg_dist);
    }
    delete ds;
    return 0;
}
