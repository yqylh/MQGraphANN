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
};

class QUERYANS {
public:
    int us;
    double recall;
    QUERYANS(int us, double recall) : us(us), recall(recall) {}
};


#endif