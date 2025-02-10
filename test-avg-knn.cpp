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
    #if DatabaseSelect > 6
        HDF5DataSet<FILETYPE> *ds = new HDF5DataSet<FILETYPE>(baseFileName);
    #else
        SIFTDataSet<FILETYPE> *ds = new SIFTDataSet<FILETYPE>(baseFileName, queryFileName, ansFileName);
    #endif 
    DataSet<FILETYPE> *dataSet = ds;


    
    delete ds;
    return 0;
}
