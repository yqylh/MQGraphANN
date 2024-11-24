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
    std::string baseFileName = "./dataset/sift-128-euclidean.hdf5";
    std::string createFileName = "./dataset/sift-128-euclidean.query";
    std::string createFileAnsName = "./dataset/sift-128-euclidean.ans";
#endif
#if DatabaseSelect == FashionMNIST
    const int K = 100;
    const int D = 784;
    #define FILETYPE float
    int maxbaseNum = 60000;
    std::string baseFileName = "./dataset/fashion-mnist-784-euclidean.hdf5";
    std::string createFileName = "./dataset/fashion-mnist-784-euclidean.query";
    std::string createFileAnsName = "./dataset/fashion-mnist-784-euclidean.ans";
#endif

#if DatabaseSelect == MNIST
    const int K = 100;
    const int D = 784;
    #define FILETYPE float
    int maxbaseNum = 60000;
    std::string baseFileName = "./dataset/mnist-784-euclidean.hdf5";
    std::string createFileName = "./dataset/mnist-784-euclidean.query";
    std::string createFileAnsName = "./dataset/mnist-784-euclidean.ans";
#endif

#if DatabaseSelect == GIST
    const int K = 100;
    const int D = 960;
    #define FILETYPE float
    int maxbaseNum = 1000000;
    std::string baseFileName = "./dataset/gist-960-euclidean.hdf5";
    std::string createFileName = "./dataset/gist-960-euclidean.query";
    std::string createFileAnsName = "./dataset/gist-960-euclidean.ans";
#endif

#if DatabaseSelect == LASTFM
    const int K = 100;
    const int D = 65;
    #define FILETYPE float
    int maxbaseNum = 292385;
    std::string baseFileName = "./dataset/lastfm-64-dot.hdf5";
    std::string createFileName = "./dataset/lastfm-64-dot.query";
    std::string createFileAnsName = "./dataset/lastfm-64-dot.ans";
#endif

#if DatabaseSelect == NYTIMES
    const int K = 100;
    const int D = 256;
    #define FILETYPE float
    int maxbaseNum = 290000;
    std::string baseFileName = "./dataset/nytimes-256-angular.hdf5";
    std::string createFileName = "./dataset/nytimes-256-angular.query";
    std::string createFileAnsName = "./dataset/nytimes-256-angular.ans";
#endif

#if DatabaseSelect == GLOVE25
    const int K = 100;
    const int D = 25;
    #define FILETYPE float
    int maxbaseNum = 1183514;
    std::string baseFileName = "./dataset/glove-25-angular.hdf5";
    std::string createFileName = "./dataset/glove-25-angular.query";
    std::string createFileAnsName = "./dataset/glove-25-angular.ans";
#endif 

#if DatabaseSelect == GLOVE100
    const int K = 100;
    const int D = 100;
    #define FILETYPE float
    int maxbaseNum = 1183514;
    std::string baseFileName = "./dataset/glove-100-angular.hdf5";
    std::string createFileName = "./dataset/glove-100-angular.query";
    std::string createFileAnsName = "./dataset/glove-100-angular.ans";
#endif

#endif