/**
 * DataSet.cpp
 * Inputs for processing data sets
 * Author: yqy
 * Date:2022.11.8
 * Main structure:Item records information about each vector, DataSet records information about the dataset
 * DataSet is an abstract class, its derived classes are: for the processing of different datasets, the goal is to different data are processed into a uniform format
 * When using the object of the derived class will be converted to the DataSet class.
 * 
*/

#ifndef __DATASET_H__
#define __DATASET_H__
#include <cstdio>
#include <vector>
#include <cmath>
#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <thread>
#include <chrono>
#include <mutex>
#include "Config.cpp"
#include "HDF5read.cpp"
/**
 * Item
 * Used to record information about each vector
 * vector: vector
 * edge : the KNN/NSW graph of the vector
 * Provide overloaded operator : [] -(square of Euclidean distance)
*/
template <typename T>
class Item {
public: 
    Item(){
        edge.clear();
    }
    Item(int lenth) : vectors(lenth){
        edge.clear();
    }
    ~Item(){}
    std::vector<T> vectors; // vectors
    std::vector<int> edge; // edge
    std::vector<int> nsgEdge; // nsg edge
    // HNSW 
    std::vector<std::vector<int> > layerEdge; 
    std::vector<std::vector<int> > nsgLayerEdge; 
    int layerNum; 
    T &operator[](int i) { return vectors[i]; }
    // cluster
    int clusterId; 
    double operator-(Item<T> &item) {
        double sum = 0;
        for (int i = 0; i < vectors.size(); i++) {
            sum += (vectors[i] - item.vectors[i]) * (vectors[i] - item.vectors[i]);
        }
        return sum;
    }
    double operator*(Item<T> &item) {
        double sum = 0;
        for (int i = 0; i < vectors.size(); i++) {
            sum += vectors[i] * item.vectors[i];
        }
        return sum;
    }
    double length() {
        double sum = 0;
        for (int i = 0; i < vectors.size(); i++) {
            sum += vectors[i] * vectors[i];
        }
        return sqrt(sum);
    }
    inline void print() {
        for (auto & item : vectors) {
            std::cout << item << " ";
        }
        std::cout << std::endl;
    }
};
/**
 * Used to record information about each cluster
 * clusterId:id of the cluster
 * clusterItem:vector of clusters
 * clusterCenter:center of cluster
 * clusterNum:number of clusters

*/
class ClusterItem {
public:
    int clusterId;
    std::vector<int> clusterItem;
    int clusterNum;
    Item<FILETYPE> clusterCenter;
    ClusterItem() {
        clusterId = -1;
        clusterItem.clear();
        clusterNum = 0;
    }
    ~ClusterItem() {}
};

template <typename T>
class DataSet{
public:
    DataSet() {
        dimension = -1;
    }
    ~DataSet() {}
    int dimension; 
    std::vector<Item<T>> baseData; 
    std::vector<Item<int>> ansData; 
    std::vector<Item<T>> queryData; 
    int maxLayerNum; 
    int beginVector; 
    // int getLevel() {
    //     int level = 0;
    //     while (level < maxLayerNum && double (rand() % 1000) / 1000 < HNSWProbability) {
    //         level++;
    //     }
    //     return level;
    // }
};


template <typename T>
class HDF5DataSet: public DataSet<T>{
public:
    HDF5DataSet(std::string baseFile) : DataSet<T>(), baseFileName(baseFile) {
        readBaseData();
        readQueryData();
        readAnsData();
        clusterData();
    }
    ~HDF5DataSet(){}
private:
    std::string baseFileName;
    void readBaseData() {
        // std::cout << "loading base data:   " << std::endl;
        auto base = read_dataset<T>(baseFileName, "train");
        for (auto & item : base) {
            this->baseData.emplace_back(item.size());
            this->baseData.back().vectors = item;
        }
        this->baseData.shrink_to_fit();
        this->dimension = D;
        // std::cout << "successed load " << this->baseData.size() << " vectors" << std::endl;
        // std::cout << "load base data done!" << std::endl;
        // std::cout << "==========================================" << std::endl;
    }
    void readQueryData() {
        // std::cout << "loading query data:   " << std::endl;
        auto test = read_dataset<T>(baseFileName, "test");
        for (auto & item : test) {
            this->queryData.emplace_back(item.size());
            this->queryData.back().vectors = item;
        }
        this->queryData.shrink_to_fit();
        // std::cout << "successed load " << this->queryData.size() << " vectors" << std::endl;
        // std::cout << "load query data done!" << std::endl;
        // std::cout << "==========================================" << std::endl;
    }
    void readAnsData() {
        // std::cout << "loading ans data:   " << std::endl;
        auto neighbors = read_dataset<int>(baseFileName, "neighbors");
        for (auto & item : neighbors) {
            this->ansData.emplace_back(item.size());
            this->ansData.back().vectors = item;
        }
        this->ansData.shrink_to_fit();
        // std::cout << "successed load " << this->ansData.size() << " vectors" << std::endl;
        // std::cout << "load ans data done!" << std::endl;
        // std::cout << "==========================================" << std::endl;
    }
    void createQueryData() {
        std::ofstream out(createFileName);
        std::vector<float> eps(D, 0.0);
        int queryNum = this->queryData.size();
        int baseNum = this->baseData.size();
        // std::cout << "queryNum: " << queryNum << std::endl;
        // 计算每个维度的平均值 / 100;
        for (auto & item : this->baseData) {
            for (int i = 0; i < D; i++) {
                eps[i] += item[i] / baseNum / 100.0;
            }
        }
        // for (int i = 0; i < D; i++) {
        //     std::cout << eps[i] << " ";
        // }
        // std::cout << std::endl;
        // 生成query数据
        // 随机十个起始点
        for (int i = 0; i < 10; i++) {
            int randomId = rand() % queryNum;
            auto Item = this->queryData[randomId];
            // 每个点在周围生成 queryNum / 10 个点
            for (int j = 0; j < queryNum / 10; j++) {
                for (int j = 0; j < D; j++) {
                    // 生成一个随机数，范围在 0 ~ item + eps
                    float temp = Item.vectors[j] + rand() % 1000 / 1000.0 * eps[j];
                    out << temp << " ";
                }
                out << std::endl;
            }
        }
        out.close();
    }
    void createAnsData() {
        std::ofstream out(createFileAnsName);
        std::vector<std::vector<int> > out_vec (this->queryData.size(), std::vector<int>(K, 0));
        #pragma omp parallel for
        for (int i = 0; i < this->queryData.size(); i++) {
            std::vector<std::pair<double, int>> ans;
            for (int j = 0; j < this->baseData.size(); j++) {
                ans.emplace_back(this->queryData[i] - this->baseData[j], j);
            }
            std::sort(ans.begin(), ans.end());
            for (int j = 0; j < K; j++) {
                out_vec[i][j] = ans[j].second;
            }
        }
        for (int i = 0; i < this->queryData.size(); i++) {
            for (int j = 0; j < K; j++) {
                out << out_vec[i][j] << " ";
            }
            out << std::endl;
        }
        out.close();
    }
    void clusterData() {
        std::ifstream in(createFileName);
        if (!in.is_open()) {
            createQueryData();
            in.open(createFileName);
        }
        // read query data
        for (auto & item : this->queryData) {
            for (auto & num : item.vectors) {
                in >> num;
            }
        }
        in.close();
        // ans
        in.open(createFileAnsName);
        if (!in.is_open()) {
            createAnsData();
            in.open(createFileAnsName);
        }
        for (auto & item : this->ansData) {
            for (auto & num : item.vectors) {
                in >> num;
            }
        }
        // for (auto & num : this->queryData[0].vectors) {
        //     std::cout << num << " ";
        // }
        // std::cout << std::endl;
        // for (auto & num : this->ansData[0].vectors) {
        //     std::cout << num << " ";
        // }
        // std::cout << std::endl;
        in.close();
    }
};

class QUERYANS {
public:
    int us;
    double recall;
    QUERYANS(int us, double recall) : us(us), recall(recall) {}
};


#endif