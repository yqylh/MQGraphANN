#include "lib/DataSet.cpp"
#include "lib/Config.cpp"
#include "./res/hnswlib/hnswlib/hnswlib.h"
#include "./res/kmeans/kmeans.cpp"

std::string solveName(int dataset, int M, int ef_construction, int zero) {
    std::string index = "./dataset/index/"
        + std::to_string(dataset) 
        + "_" + std::to_string(M) 
        + "_" + std::to_string(ef_construction) 
        + "_" + std::to_string(zero) + ".bin";
    return index;
}
std::vector<float> norm_vector(std::vector<float> data) {
    std::vector<float> norm_array;
    float norm = 0.0f;
    for (auto & item : data)
        norm += item * item;
    norm = 1.0f / (sqrtf(norm) + 1e-30f);
    for (auto & item : data)
        norm_array.push_back(item * norm);
    return norm_array;
}

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
    DataSet<FILETYPE> *dataSet = new HDF5DataSet<FILETYPE>(baseFileName);
    // Initing index
    hnswlib::L2Space space(D); // D Dimension of the elements
    hnswlib::HierarchicalNSW<float>* alg_hnsw;
    // cosin space
    hnswlib::InnerProductSpace space_cos(D);

    std::string index;
    #ifdef ZERO
        index = solveName(DatabaseSelect, M, ef_construction, 1);
    #else
        index = solveName(DatabaseSelect, M, ef_construction, 0);
    #endif
    std::ifstream file(index);
    // std::cout << "index: " << index << std::endl;
    if (file.is_open()) {
        if (DatabaseSelect >= 10) {
            alg_hnsw = new hnswlib::HierarchicalNSW<float>(&space_cos, index.c_str());
        } else  alg_hnsw = new hnswlib::HierarchicalNSW<float>(&space, index.c_str());
    } else {
        // maxbaseNum-> Maximum number of elements, should be known beforehand
        if (DatabaseSelect >= 10) {
            alg_hnsw = new hnswlib::HierarchicalNSW<float>(&space_cos, maxbaseNum, M, ef_construction);
        } else alg_hnsw = new hnswlib::HierarchicalNSW<float>(&space, maxbaseNum, M, ef_construction);
        for (int i = 0; i < dataSet->baseData.size(); i++) {
            if (DatabaseSelect == 5) {
                std::vector<float> temp;
                for (auto & item : dataSet->baseData[i].vectors) {
                    temp.push_back(item);
                }
                alg_hnsw->addPoint(temp.data(), i);
            } else if (DatabaseSelect >= 10) {
                std::vector<float> temp = norm_vector(dataSet->baseData[i].vectors);
                alg_hnsw->addPoint(temp.data(), i);
            }else
                alg_hnsw->addPoint(dataSet->baseData[i].vectors.data(), i);
        }
        alg_hnsw->saveIndex(index.c_str());
    }
    // Query the elements for themselves and measure recall
    float correct = 0;
    double allTime = 0;
    #ifdef CLUSTER
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
        KMeans kmeans(num_clusters);
        kmeans.init(points);
        kmeans.run();
        for (int i = 0; i < num_clusters; i++) {
            clusters.emplace_back();
            clusters[i].clusterId = i;
            auto & temp = kmeans.getMeans()[i].data_;
            for (auto & item : temp) {
                clusters[i].clusterCenter.vectors.push_back(item);
            }
        }
        for (auto & item : kmeans.getPoints()) {
            clusters[item.cluster_].clusterItem.push_back(item.id);
            clusters[item.cluster_].clusterNum++;
        }
        for (auto & cluster : clusters) {
            // sort with - 
            std::sort(cluster.clusterItem.begin(), cluster.clusterItem.end(), [&](int a, int b) {
                return dataSet->queryData[a] - cluster.clusterCenter < dataSet->queryData[b] - cluster.clusterCenter;
            });
        }
        std::vector<QUERYANS> queryAns;
        std::mutex add;
        // #pragma omp parallel for num_threads(THREAD_CONFIG)
        for (auto & cluster : clusters) {
            std::vector<int> beginVector;
            for (auto & i : cluster.clusterItem) {
                std::priority_queue<std::pair<float, hnswlib::labeltype>> result;
                auto start = std::chrono::steady_clock::now();
                if (DatabaseSelect == 5) {
                    std::vector<float> temp;
                    for (auto & item : dataSet->queryData[i].vectors) {
                        temp.push_back(item);
                    }
                    result = alg_hnsw->searchKnn(temp.data(), K, nullptr, beginVector);
                } else if (DatabaseSelect >= 10) {
                    std::vector<float> temp = norm_vector(dataSet->queryData[i].vectors);
                    result = alg_hnsw->searchKnn(temp.data(), K, nullptr, beginVector);
                } else {
                    result = alg_hnsw->searchKnn(dataSet->queryData[i].vectors.data(), K, nullptr, beginVector);
                }
                auto end = std::chrono::steady_clock::now();
                add.lock();
                allTime += (end - start)/ 1us;
                if (beginVector.size() < CLUSTER_CORE) beginVector.push_back(result.top().second);
                while (!result.empty()) {
                    int ans = result.top().second;
                    result.pop();
                    for (auto & ansItem : dataSet->ansData[i].vectors) {
                        if (ans == ansItem) {
                            correct++;
                            break;
                        }
                    }
                }
                add.unlock();
            }
        }
    #else
        for (int i = 0; i < dataSet->queryData.size(); i++) {
            std::priority_queue<std::pair<float, hnswlib::labeltype>> result;
            auto start = std::chrono::steady_clock::now();
            if (DatabaseSelect == 5) {
                std::vector<float> temp;
                for (auto & item : dataSet->queryData[i].vectors) {
                    temp.push_back(item);
                }
                result = alg_hnsw->searchKnn(temp.data(), K);
            } else if (DatabaseSelect >= 10) {
                std::vector<float> temp = norm_vector(dataSet->queryData[i].vectors);
                result = alg_hnsw->searchKnn(temp.data(), K);
            } else
                result = alg_hnsw->searchKnn(dataSet->queryData[i].vectors.data(), K);
            auto end = std::chrono::steady_clock::now();
            allTime += (end - start)/ 1us;
            while (!result.empty()) {
                int ans = result.top().second;
                result.pop();
                for (auto & ansItem : dataSet->ansData[i].vectors) {
                    if (ans == ansItem) {
                        correct++;
                        break;
                    }
                }
            }
        }
    #endif
    float recall = correct / (dataSet->queryData.size() * K);
    std::cout
        << "Dataset=" << DatabaseSelect << "\t"
        << "M=" << M << "\t"
        << "ef_construction=" << ef_construction << "\t"
        << "CLUSTER_NUM=" << CLUSTER_NUM << "\t"
        << "CLUSTER_CORE=" << CLUSTER_CORE << "\t"
        << "Recall=" << recall << "\t" 
        << "avgTime=" << allTime / dataSet->queryData.size() << "us" 
        << std::endl;
    
    delete alg_hnsw;
    delete ds;
    return 0;
}
