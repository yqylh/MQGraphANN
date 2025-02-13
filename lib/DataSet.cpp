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
#include "./HDF5read.cpp"

template <typename T>
class Item {
public: 
    Item(){}
    Item(int lenth) : vectors(lenth){}
    ~Item(){}
    std::vector<T> vectors; // vectors
    T &operator[](int i) { return vectors[i]; }
    float operator-(Item<T> &item) {
        float sum = 0;
        for (int i = 0; i < vectors.size(); i++) {
            sum += (vectors[i] - item.vectors[i]) * (vectors[i] - item.vectors[i]);
        }
        return sum;
    }
    float operator*(Item<T> &item) {
        float sum = 0;
        for (int i = 0; i < vectors.size(); i++) {
            sum += vectors[i] * item.vectors[i];
        }
        return sum;
    }
    float length() {
        float sum = 0;
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
};

// 通用的写 fvecs 文件的函数模板
// 对于 fvecs 格式，每个向量首先写入一个 int 型数（维度），接着写入该向量所有 float 数据
template <typename T>
void writeFvecs(const std::string &filename, const std::vector<Item<T>> &data) {
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs) {
        std::cerr << "Error: cannot open file " << filename << " for writing." << std::endl;
        return;
    }
    for (const auto &item : data) {
        int d = static_cast<int>(item.vectors.size());
        // 写入维度信息
        ofs.write(reinterpret_cast<const char*>(&d), sizeof(int));
        // 写入向量数据
        ofs.write(reinterpret_cast<const char*>(item.vectors.data()), d * sizeof(T));
    }
    ofs.close();
}
template <typename T>
class HDF5DataSet: public DataSet<T>{
public:
    HDF5DataSet(std::string baseFile) : DataSet<T>(), baseFileName(baseFile) {
        readBaseData();
        readQueryData();
        readAnsData();
    }
    ~HDF5DataSet(){}
    void convertToFvecs(const std::string &baseOutFile, const std::string &queryOutFile) {
        // 将 base 数据写入 fvecs 文件
        writeFvecs<T>(baseOutFile, this->baseData);
        // 将 query 数据写入 fvecs 文件
        writeFvecs<T>(queryOutFile, this->queryData);
        std::cout << "Successfully converted base and query data to fvecs format." << std::endl;
    }
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
    }
};


#endif