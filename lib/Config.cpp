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
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <queue>
#include <functional>
#include <climits>
#include <stack>
#include <map>
#include <mutex>
#include <omp.h>
#include <tuple>
using namespace std::chrono;

#define uchar unsigned char
#define THREAD_CONFIG 1

#define SIFT1M 1
#define FashionMNIST 7
#define MNIST 8
#define GIST 9
#define LASTFM 10
#define NYTIMES 11
#define GLOVE25 12
#define GLOVE100 13

int M;                 // Tightly connected with internal dimensionality of the data strongly affects the memory consumption
int ef;  // Controls index search speed/build speed tradeoff
int cluster_num;        // Number of clusters
int m_circle;          // Number of circles
int m_sparse;         // Number of sparse and core
int search_center;    // Whether to search the center point

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
    std::string baseFileName = "./dataset/sift-128-euclidean.hdf5";
#endif
#if DatabaseSelect == FashionMNIST
    const int K = 100;
    const int D = 784;
    int maxbaseNum = 60000;
    std::string baseFileName = "./dataset/fashion-mnist-784-euclidean.hdf5";
#endif

#if DatabaseSelect == MNIST
    const int K = 100;
    const int D = 784;
    int maxbaseNum = 60000;
    std::string baseFileName = "./dataset/mnist-784-euclidean.hdf5";
#endif

#if DatabaseSelect == GIST
    const int K = 100;
    const int D = 960;
    int maxbaseNum = 1000000;
    std::string baseFileName = "./dataset/gist-960-euclidean.hdf5";
#endif

#if DatabaseSelect == LASTFM
    const int K = 100;
    const int D = 65;
    int maxbaseNum = 292385;
    std::string baseFileName = "./dataset/lastfm-64-dot.hdf5";
#endif

#if DatabaseSelect == NYTIMES
    const int K = 100;
    const int D = 256;
    int maxbaseNum = 290000;
    std::string baseFileName = "./dataset/nytimes-256-angular.hdf5";
#endif

#if DatabaseSelect == GLOVE25
    const int K = 100;
    const int D = 25;
    int maxbaseNum = 1183514;
    std::string baseFileName = "./dataset/glove-25-angular.hdf5";
#endif 

#if DatabaseSelect == GLOVE100
    const int K = 100;
    const int D = 100;
    int maxbaseNum = 1183514;
    std::string baseFileName = "./dataset/glove-100-angular.hdf5";
#endif

template<typename... Args>
void bugs(Args... args) {
    (..., (std::cout << args << " "));
    std::cout << std::endl;
}
std::mutex pf_mtx;
std::ofstream pf_log_file("p_log");
template<typename... Args>
void pfbugs(Args... args) {
    pf_mtx.lock();
    pf_log_file << "pid=" << std::this_thread::get_id() << " t=";
    auto now = system_clock::now();
    auto now_c = system_clock::to_time_t(now);
    pf_log_file << std::ctime(&now_c) << " ";
    (..., (pf_log_file << args << " \n"));
    pf_log_file << std::endl;
    pf_mtx.unlock();
}

class Timer {
public:
    template<typename Func, typename... Args>
    static auto measure(const std::string& description, std::ofstream &tout, Func func, Args&&... args) {
        auto start = std::chrono::high_resolution_clock::now();
        
        if constexpr (std::is_same_v<std::invoke_result_t<Func, Args...>, void>) {
            // If the function returns void
            func(std::forward<Args>(args)...);
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed = end - start;
            tout <<" "<< description << "\t - Elapsed time: \t" << elapsed.count() * 1000 << "\tmirco seconds.\n";
        } else {
            // If the function returns a value
            auto result = func(std::forward<Args>(args)...);
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed = end - start;
            tout <<" "<< description << "\t - Elapsed time: \t" << elapsed.count() * 1000 << "\tmicro seconds.\n";
            return result;
        }
        std::cout << description << " - Done." << std::endl;
    }
};

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

#endif