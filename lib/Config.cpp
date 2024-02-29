#ifndef __config_CPP__
#define __config_CPP__
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

#define uchar unsigned char
int CREATEGRAPH = 0;
std::mutex *mtx;
#define THREAD_CONFIG 1

// Selecting a dataset
#define SIFT1M 1
#define SIFT1W 2
#define SIFT1B10M 3
#define SIFT1B50M 4
#define SIFT1B100M 5
#define SIFT1B200M 6
#define FashionMNIST 7
#define MNIST 8
#define GIST 9
#define LASTFM 10
#define NYTIMES 11
#define GLOVE25 12
#define GLOVE100 13
// #define DatabaseSelect SIFT1M

int CLUSTER_NUM = 400;
int CLUSTER_CORE = 2;
int M = 64;                 // Tightly connected with internal dimensionality of the data strongly affects the memory consumption
int ef_construction = 20;  // Controls index search speed/build speed tradeoff

// Dataset configuration
/**
 * D: Dimension of each vector
 * M: Maximum number of edges per vector
 * K: number of query neighbors for each vector
*/
// Index whether to crop max edges
#if DatabaseSelect == SIFT1M
    const int K = 100;
    const int D = 128;
    const int maxbaseNum = 1000000;
    #define FILETYPE float
    std::string baseFileName = "./dataset/sift1M/sift_base.fvecs";
    std::string queryFileName = "./dataset/sift1M/sift_query.fvecs";
    std::string ansFileName = "./dataset/sift1M/sift_groundtruth.ivecs";
#endif
#if DatabaseSelect == SIFT1W
    const int K = 100;
    const int D = 128;
    int maxbaseNum = 10000;
    #define FILETYPE float
    std::string baseFileName = "./dataset/sift1W/siftsmall_base.fvecs";
    std::string queryFileName = "./dataset/sift1W/siftsmall_query.fvecs";
    std::string ansFileName = "./dataset/sift1W/siftsmall_groundtruth.ivecs";
#endif

#if DatabaseSelect == SIFT1B10M
    const int K = 100;
    const int D = 128;
    #define FILETYPE uchar
    int maxbaseNum = 10000000;
    std::string baseFileName = "./dataset/sift1B/bigann_base.bvecs";
    std::string queryFileName = "./dataset/sift1B/bigann_query.bvecs";
    std::string ansFileName = "./dataset/sift1B/ans/idx_10M.ivecs";
#endif

#if DatabaseSelect == SIFT1B50M
    const int K = 100;
    const int D = 128;
    #define FILETYPE uchar
    int maxbaseNum = 50000000;
    std::string baseFileName = "./dataset/sift1B/bigann_base.bvecs";
    std::string queryFileName = "./dataset/sift1B/bigann_query.bvecs";
    std::string ansFileName = "./dataset/sift1B/ans/idx_50M.ivecs";
#endif
#if DatabaseSelect == SIFT1B100M
    const int K = 100;
    const int D = 128;
    #define FILETYPE uchar
    int maxbaseNum = 100000000;
    std::string baseFileName = "./dataset/sift1B/bigann_base.bvecs";
    std::string queryFileName = "./dataset/sift1B/bigann_query.bvecs";
    std::string ansFileName = "./dataset/sift1B/ans/idx_100M.ivecs";
#endif
#if DatabaseSelect == SIFT1B200M
    const int K = 100;
    const int D = 128;
    #define FILETYPE uchar
    int maxbaseNum = 200000000;
    std::string baseFileName = "./dataset/sift1B/bigann_base.bvecs";
    std::string queryFileName = "./dataset/sift1B/bigann_query.bvecs";
    std::string ansFileName = "./dataset/sift1B/ans/idx_200M.ivecs";
#endif
#if DatabaseSelect == FashionMNIST
    const int K = 100;
    const int D = 784;
    #define FILETYPE float
    int maxbaseNum = 60000;
    std::string baseFileName = "./dataset/hdf5/fashion-mnist-784-euclidean.hdf5";
#endif

#if DatabaseSelect == MNIST
    const int K = 100;
    const int D = 784;
    #define FILETYPE float
    int maxbaseNum = 60000;
    std::string baseFileName = "./dataset/hdf5/mnist-784-euclidean.hdf5";
#endif

#if DatabaseSelect == GIST
    const int K = 100;
    const int D = 960;
    #define FILETYPE float
    int maxbaseNum = 1000000;
    std::string baseFileName = "./dataset/hdf5/gist-960-euclidean.hdf5";
#endif

#if DatabaseSelect == LASTFM
    const int K = 100;
    const int D = 65;
    #define FILETYPE float
    int maxbaseNum = 292385;
    std::string baseFileName = "./dataset/hdf5/lastfm-64-dot.hdf5";
#endif

#if DatabaseSelect == NYTIMES
    const int K = 100;
    const int D = 256;
    #define FILETYPE float
    int maxbaseNum = 290000;
    std::string baseFileName = "./dataset/hdf5/nytimes-256-angular.hdf5";
#endif

#if DatabaseSelect == GLOVE25
    const int K = 100;
    const int D = 25;
    #define FILETYPE float
    int maxbaseNum = 1183514;
    std::string baseFileName = "./dataset/hdf5/glove-25-angular.hdf5";
#endif 

#if DatabaseSelect == GLOVE100
    const int K = 100;
    const int D = 100;
    #define FILETYPE float
    int maxbaseNum = 1183514;
    std::string baseFileName = "./dataset/hdf5/glove-100-angular.hdf5";
#endif

#endif