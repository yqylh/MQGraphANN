#ifndef __CircleIndex__CPP__
#define __CircleIndex__CPP__

#include "Config.cpp"
#include "DataSet.cpp"
#include "../res/kmeans/kmeans.cpp"

class CircleIndex {
public:
    std::string baseFileName;
    int M;
    int ef;
    int cluster_num;
    int m_circle;
    int m_sparse;
    DataSet<float> *dataSet;
    CircleIndex(std::string baseFileName, int M, int ef, int cluster_num, int m_circle, int m_sparse) {
        this->baseFileName = baseFileName;
        this->M = M;
        this->ef = ef;
        this->cluster_num = cluster_num;
        this->m_circle = m_circle;
        this->m_sparse = m_sparse;
        dataSet = new HDF5DataSet<float>(baseFileName);
    }
    ~CircleIndex() {
        delete dataSet;
    }

};

#endif