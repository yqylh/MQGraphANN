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

    int num_clusters = dataSet->queryData.size() / CLUSTER_NUM;
    std::vector<ClusterItem> clusters;
    std::vector<Point> points;
    for (int i = 0; i < dataSet->queryData.size(); i++) {
        std::vector<double> temp;
        for (auto & item : dataSet->queryData[i].vectors) {
            temp.push_back(item);
        }
        Point p(temp);
        p.id = i;
        points.push_back(p);
    }
    auto begin = std::chrono::high_resolution_clock::now();
    KMeans kmeans(num_clusters);
    kmeans.init(points);
    kmeans.run();
    auto end = std::chrono::high_resolution_clock::now();
    std::string file = "time/" + std::to_string(DatabaseSelect);
    std::ofstream out(file, std::ios::app);
    out << CLUSTER_NUM << " " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << std::endl;
    out.close();
    delete ds;
    return 0;
}
