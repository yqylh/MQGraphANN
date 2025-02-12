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
    if (argc != 3) {
        std::cout << "Usage: ./main <M> <ef_construction>" << std::endl;
        return 0;
    }
    M = atoi(argv[1]);
    ef_construction = atoi(argv[2]);
    std::ios::sync_with_stdio(false);
    std::cin.tie(0);
    DataSet<float> *dataSet = new HDF5DataSet<float>(baseFileName);

    hnswlib::L2Space space(D); 
    hnswlib::HierarchicalNSW<float>* alg_hnsw;
    hnswlib::InnerProductSpace space_cos(D);

    std::string index;
    #ifdef ZERO
        index = solveName(DatabaseSelect, M, ef_construction, 1);
    #else
        index = solveName(DatabaseSelect, M, ef_construction, 0);
    #endif
    std::ifstream file(index);
    if (file.is_open()) {
        if (DatabaseSelect >= 10) {
            alg_hnsw = new hnswlib::HierarchicalNSW<float>(&space_cos, index.c_str());
        } else  alg_hnsw = new hnswlib::HierarchicalNSW<float>(&space, index.c_str());
    } else {
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
    float recall = correct / (dataSet->queryData.size() * K);
    std::ofstream out;
    #ifdef ZERO
        out.open("./result/create-nsw-" + std::to_string(DatabaseSelect), std::ios::app);
    #else
        out.open("./result/create-hnsw-" + std::to_string(DatabaseSelect), std::ios::app);
    #endif
    out
        << "Dataset=" << DatabaseSelect << "\t"
        << "M=" << M << "\t"
        << "ef_construction=" << ef_construction << "\t"
        << "Recall=" << recall << "\t" 
        << "avgTime=" << allTime / dataSet->queryData.size() << "us" 
        << std::endl;
    out.close();
    delete alg_hnsw;
    delete dataSet;
    return 0;
}
