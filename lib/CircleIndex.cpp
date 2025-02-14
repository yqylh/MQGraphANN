#ifndef __CircleIndex__CPP__
#define __CircleIndex__CPP__

#include "Config.cpp"
#include "DataSet.cpp"
#include "../res/kmeans/kmeans.cpp"
#include "../res/hnswlib/hnswlib/hnswlib.h"

using index_t = hnswlib::HierarchicalNSW<float>*;
class CircleIndex {
public:
    // info
    std::ofstream logFile;
    std::string bothName;
    // 全局参数
    std::string baseFileName;
    int ef; // queue max size
    int cluster_num;
    int m_circle;
    int m_sparse; 
    int search_center;
    int D; // 维度
    int N; // 数据集大小 
    HDF5DataSet<float> *dataSet;
    // 聚类
    std::vector<int> cluster_id; // 每个数据点的簇id 范围[0, cluster_num)
    std::vector<std::vector<std::pair<int, float> > > cluster; // 每个簇的数据点id 和距离, 按照距离排序
    // 下列变量需要输出到索引中
    std::vector<Item<float>> center; // 每个簇的中心 && 环中心节点 list
    std::vector<std::vector<int>> circle;// 稠密区域
    std::vector<int> sparse; // 稀疏区域
    // 索引
    std::vector<index_t > circleIndex; // 环索引
    index_t sparseIndex; // 稀疏索引
    index_t centerIndex; // 中心索引
    hnswlib::L2Space *sparseSpaceL2;
    hnswlib::InnerProductSpace *sparseSpaceIP;

    hnswlib::L2Space *centerSpaceL2;
    hnswlib::InnerProductSpace *centerSpaceIP;
    std::vector<hnswlib::L2Space> circleSpaceL2;
    std::vector<hnswlib::InnerProductSpace> circleSpaceIP;

    CircleIndex(std::string baseFileName, int ef, int m_circle, int m_sparse, int search_center) {
        // 初始化全局参数
        this->baseFileName = baseFileName;
        this->ef = ef;
        this->cluster_num = std::sqrt(maxbaseNum);
        this->m_circle = m_circle;
        this->m_sparse = m_sparse;
        this->search_center = search_center;
        // 初始化数据集
        dataSet = new HDF5DataSet<float>(baseFileName);
        D = dataSet->dimension;
        N = dataSet->baseData.size();
        // 初始化聚类
        cluster_id.resize(N);
        cluster.resize(cluster_num);
        center.resize(cluster_num);
        // 初始化日志
        logFile.open("./log/ds=" + std::to_string(DatabaseSelect) + "_ef_" + std::to_string(ef) + "_m_circle_" + std::to_string(m_circle) + "_m_sparse_" + std::to_string(m_sparse) + ".log", std::ios::app);
        bothName = "./dataset/circle/" + std::to_string(DatabaseSelect) + "_ef_" + std::to_string(ef) + "_m_circle_" + std::to_string(m_circle) + "_m_sparse_" + std::to_string(m_sparse);
        // 初始化索引
        circleIndex.resize(cluster_num);
        sparseSpaceL2 = new hnswlib::L2Space(D);
        centerSpaceL2 = new hnswlib::L2Space(D);
        sparseSpaceIP = new hnswlib::InnerProductSpace(D);
        centerSpaceIP = new hnswlib::InnerProductSpace(D);
        for (int i = 0; i < cluster_num; i++) {
            circleSpaceL2.push_back(hnswlib::L2Space(D));
            circleSpaceIP.push_back(hnswlib::InnerProductSpace(D));
        }
    }
    ~CircleIndex() {
        delete dataSet;
        logFile.close();
        for (int i = 0; i < cluster_num; i++) {
            delete circleIndex[i];
        }
        delete sparseIndex;
        delete centerIndex;
        delete sparseSpaceL2;
        delete centerSpaceL2;
        delete sparseSpaceIP;
        delete centerSpaceIP;
    }
    void paralleKmeans() {
        // 设置最大迭代次数
        const int max_iters = 20;
        
        // 初始化聚类中心：这里简单地用数据集中的前 cluster_num 个点作为初始中心
        for (int c = 0; c < cluster_num; c++) {
            // 确保中心的维度与数据一致
            center[c] = dataSet->baseData[c];
        }
    
        bool converged = false;
        for (int iter = 0; iter < max_iters && !converged; iter++) {
            // ---------------------- Assignment Step ----------------------
            // 为每个数据点分配离它最近的聚类中心
            #pragma omp parallel for schedule(static)
            for (int i = 0; i < N; i++) {
                const std::vector<float>& point = dataSet->baseData[i].vectors;
                int best_cluster = 0;
                float best_dist = std::numeric_limits<float>::max();
                for (int c = 0; c < cluster_num; c++) {
                    float dist = 0.0f;
                    const std::vector<float>& center_vec = center[c].vectors;
                    // 使用 OpenMP SIMD 指令优化内循环
                    #pragma omp simd reduction(+: dist)
                    for (int k = 0; k < D; k++) {
                        float diff = point[k] - center_vec[k];
                        dist += diff * diff;
                    }
                    if (dist < best_dist) {
                        best_dist = dist;
                        best_cluster = c;
                    }
                }
                cluster_id[i] = best_cluster;
            }
    
            // ---------------------- Update Step ----------------------
            // 为避免锁的开销，采用 per-thread 的局部累加器
            int num_threads = omp_get_max_threads();
            std::vector<std::vector<float>> local_sums(num_threads, std::vector<float>(cluster_num * D, 0.0f));
            std::vector<std::vector<int>> local_counts(num_threads, std::vector<int>(cluster_num, 0));
    
            #pragma omp parallel
            {
                int tid = omp_get_thread_num();
                #pragma omp for schedule(static)
                for (int i = 0; i < N; i++) {
                    int cid = cluster_id[i];
                    const std::vector<float>& point = dataSet->baseData[i].vectors;
                    int base_idx = cid * D;
                    for (int k = 0; k < D; k++) {
                        local_sums[tid][base_idx + k] += point[k];
                    }
                    local_counts[tid][cid] += 1;
                }
            }
    
            // 汇总所有线程的局部累加器
            std::vector<std::vector<float>> global_sums(cluster_num, std::vector<float>(D, 0.0f));
            std::vector<int> global_counts(cluster_num, 0);
            for (int t = 0; t < num_threads; t++) {
                for (int c = 0; c < cluster_num; c++) {
                    int base_idx = c * D;
                    global_counts[c] += local_counts[t][c];
                    for (int k = 0; k < D; k++) {
                        global_sums[c][k] += local_sums[t][base_idx + k];
                    }
                }
            }
    
            // 根据累加的和更新聚类中心，并检测收敛性
            converged = true;
            for (int c = 0; c < cluster_num; c++) {
                if (global_counts[c] > 0) {
                    std::vector<float> new_center(D, 0.0f);
                    for (int k = 0; k < D; k++) {
                        new_center[k] = global_sums[c][k] / global_counts[c];
                    }
                    // 计算新旧中心间的差异
                    float diff = 0.0f;
                    const std::vector<float>& old_center = center[c].vectors;
                    #pragma omp simd reduction(+: diff)
                    for (int k = 0; k < D; k++) {
                        float d = new_center[k] - old_center[k];
                        diff += d * d;
                    }
                    if (diff > 1e-6f) { // 阈值可根据需求调整
                        converged = false;
                    }
                    center[c].vectors = new_center;
                } else {
                    // 若某个簇没有分配到任何点，则随机选择一个数据点作为中心
                    int random_index = rand() % N;
                    center[c] = dataSet->baseData[random_index];
                    converged = false;
                }
            }
        }
    
        // ---------------------- 后处理：构建聚类结果 ----------------------
        // 清空之前保存的聚类结果
        for (int c = 0; c < cluster_num; c++) {
            cluster[c].clear();
        }
        // 为每个数据点计算其与对应聚类中心的距离，并加入结果
        #pragma omp parallel for schedule(static)
        for (int i = 0; i < N; i++) {
            int cid = cluster_id[i];
            float dist = 0.0f;
            const std::vector<float>& point = dataSet->baseData[i].vectors;
            const std::vector<float>& center_vec = center[cid].vectors;
            #pragma omp simd reduction(+: dist)
            for (int k = 0; k < D; k++) {
                float d = point[k] - center_vec[k];
                dist += d * d;
            }
            // 使用 critical 区域避免多个线程同时写入同一簇的数据
            #pragma omp critical
            {
                cluster[cid].push_back({ i, dist });
            }
        }
        // 对每个簇内的数据点按距离升序排序
        for (int c = 0; c < cluster_num; c++) {
            std::sort(cluster[c].begin(), cluster[c].end(),
                      [](const std::pair<int, float>& a, const std::pair<int, float>& b) {
                          return a.second < b.second;
                      });
        }
        // 输出聚类结果
        for (int c = 0; c < cluster_num; c++) {
            logFile << "cluster " << c << " size: " << cluster[c].size() << std::endl;
        }
    }

    void Delineation() {
        // 预先申请：每个簇内保存各节点的 id 以及计算得到的 kth-NN 距离
        std::vector<std::vector<int>> cluster_ids(cluster_num);
        std::vector<std::vector<float>> kth_distances(cluster_num);
        circle.clear();
        circle.resize(cluster_num);
        sparse.clear();

        // 从 paralleKmeans() 得到的聚类结果 cluster，提取每个簇的节点 id
        for (int c = 0; c < cluster_num; c++) {
            int cluster_size = cluster[c].size();
            cluster_ids[c].resize(cluster_size);
            kth_distances[c].resize(cluster_size, 0.0f);
            for (int i = 0; i < cluster_size; i++) {
                cluster_ids[c][i] = cluster[c][i].first;
            }
        }
        
        #pragma omp parallel for schedule(dynamic)
        for (int c = 0; c < cluster_num; c++) {
            double total_sum = 0.0;
            float global_threshold = 0.0f;
            // 1. 对于每个簇，计算簇内每个数据点到其他数据点的 k-NN 距离
            int cluster_size = static_cast<int>(cluster_ids[c].size());
            int k = std::max(1, static_cast<int>(std::sqrt(cluster_size))); // k = sqrt(n)
            for (int i = 0; i < cluster_size; i++) {
                int node_i = cluster_ids[c][i];
                const std::vector<float>& vec_i = dataSet->baseData[node_i].vectors;
                if (cluster_size == 1) continue;
                // 计算当前节点与簇内其他点之间的距离
                std::vector<float> distances;
                distances.reserve(cluster_size - 1);
                for (int j = 0; j < cluster_size; j++) {
                    if (i == j) continue;
                    int node_j = cluster_ids[c][j];
                    const std::vector<float>& vec_j = dataSet->baseData[node_j].vectors;
                    float sq_dist = 0.0f;
                    // 使用 SIMD 指令加速内层距离计算
                    #pragma omp simd reduction(+: sq_dist)
                    for (int d = 0; d < D; d++) {
                        float diff = vec_i[d] - vec_j[d];
                        sq_dist += diff * diff;
                    }
                    distances.push_back(std::sqrt(sq_dist));
                }
                // 取 distances 数组中前 k 小的值 的 平均值作为该节点的 kth-NN 距离
                std::sort(distances.begin(), distances.end());
                float sum = 0.0f;
                for (int j = 0; j < k; j++) sum += distances[j];
                kth_distances[c][i] = sum / k;
                total_sum += kth_distances[c][i];
            }
            // 2. 对于每个簇，计算平均距离的平均值，作为全局阈值
            global_threshold = total_sum / cluster_size;
            // 3. 对于小于阈值的节点，划分到稠密区域；其余划分到稀疏区域
            for (int i = 0; i < cluster_size; i++) {
                if (kth_distances[c][i] < global_threshold) {
                    // 稠密区域
                    circle[c].push_back(cluster_ids[c][i]);
                } else {
                    // 稀疏区域
                    #pragma omp critical
                    {
                        sparse.push_back(cluster_ids[c][i]);
                    }
                }
            }
            #pragma omp critical
            {
                logFile << "cluster " << c << " global_threshold: " << global_threshold << " circle size: " << circle[c].size() << " sparse size: " << cluster_size - circle[c].size() << std::endl;
            }
        }
    }
    
    void buildCircleIndex() {
        // 对每个簇的稠密区域建立索引
        auto circleName = bothName + ".circle_";
        #pragma omp parallel for
        for (int i = 0; i < cluster_num; i++) {
            if (DatabaseSelect >= 10) {
                circleIndex[i] = new hnswlib::HierarchicalNSW<float>(&circleSpaceIP[i], circle[i].size(), m_circle, ef);
            } else {
                circleIndex[i] = new hnswlib::HierarchicalNSW<float>(&circleSpaceL2[i], circle[i].size(), m_circle, ef);
            }
            for (auto & item : circle[i]) {
                if (DatabaseSelect >= 10) {
                    std::vector<float> temp = norm_vector(dataSet->baseData[item].vectors);
                    circleIndex[i]->addPoint(temp.data(), item);
                } else {
                    circleIndex[i]->addPoint(dataSet->baseData[item].vectors.data(), item);
                }
            }
            auto thisName = circleName + std::to_string(i) ;
            circleIndex[i]->saveIndex(thisName.c_str());
        }
    }
    void buildSparseIndex() {
        // 对每个簇的稀疏区域建立索引
        auto sparseName = bothName + ".sparse";
        if (DatabaseSelect >= 10) {
            sparseIndex = new hnswlib::HierarchicalNSW<float>(sparseSpaceIP, sparse.size(), m_sparse, ef);
        } else {
            sparseIndex = new hnswlib::HierarchicalNSW<float>(sparseSpaceL2, sparse.size(), m_sparse, ef);
        }
        for (auto & id : sparse) {
            if (DatabaseSelect >= 10) {
                std::vector<float> temp = norm_vector(dataSet->baseData[id].vectors);
                sparseIndex->addPoint(temp.data(), id);
            } else {
                sparseIndex->addPoint(dataSet->baseData[id].vectors.data(), id);
            }
        }
        sparseIndex->saveIndex(sparseName.c_str());
    }
    void buildCenterIndex() {
        // 对每个簇的中心点建立近邻图索引
        auto centerName = bothName + ".center";
        if (DatabaseSelect >= 10) {
            centerIndex = new hnswlib::HierarchicalNSW<float>(centerSpaceIP, cluster_num, m_sparse, ef);
        } else {
            centerIndex = new hnswlib::HierarchicalNSW<float>(centerSpaceL2, cluster_num, m_sparse, ef);
        }
        for (int i = 0; i < cluster_num; i++) {
            if (DatabaseSelect >= 10) {
                std::vector<float> temp = norm_vector(center[i].vectors);
                centerIndex->addPoint(temp.data(), i);
            } else {
                centerIndex->addPoint(center[i].vectors.data(), i);
            }
        }
        centerIndex->saveIndex(centerName.c_str());
    }
    void buildGraphIndex() {
        Timer::measure("建立环索引", logFile, [&] {
            buildCircleIndex();
        });
        Timer::measure("建立稀疏区域索引", logFile, [&] {
            buildSparseIndex();
        });
        Timer::measure("建立中心点索引", logFile, [&] {
            buildCenterIndex();
        });
    }

    void buildIndex() {
        auto begin = std::chrono::steady_clock::now();
        // 1. kmeans
        Timer::measure("Kmeans 分簇", logFile, [&] {
            paralleKmeans();
        });
        // 2. 划分稠密区域和稀疏区域
        Timer::measure("划分稠密区域和稀疏区域", logFile, [&] {
            Delineation();
        });
        // 3. 建立图索引
        Timer::measure("建立图索引", logFile, [&] {
            buildGraphIndex();
        });
        auto end = std::chrono::steady_clock::now();
        logFile << "总耗时: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms" << std::endl;
        saveIndex();
    }

    void saveIndex() {
        auto indexName = bothName + ".index";
        std::ofstream indexFile(indexName);
        // center
        indexFile << center.size() << std::endl;
        for (auto & item : center) {
            indexFile << item.vectors.size() << std::endl;
            for (auto & value : item.vectors) {
                indexFile << value << " ";
            }
            indexFile << std::endl;
        }
        // circle
        indexFile << circle.size() << std::endl;
        for (int i = 0; i < cluster_num; i++) {
            indexFile << circle[i].size() << std::endl;
            for (auto & item : circle[i]) {
                indexFile << item << " ";
            }
            indexFile << std::endl;
        }
        // sparse
        indexFile << sparse.size() << std::endl;
        for (auto & item : sparse) {
            indexFile << item << " ";
        }
        indexFile << std::endl;
        indexFile.close();
    }
    void loadIndex() {
        auto indexName = bothName + ".index";
        std::ifstream indexFile(indexName);
        if (!indexFile.is_open()) {
            throw runtime_error("Index file not found");
        }
        // center
        int centerSize;
        indexFile >> centerSize;
        center.resize(centerSize);
        for (int i = 0; i < centerSize; i++) {
            int lenth;
            indexFile >> lenth;
            center[i].vectors.resize(lenth);
            for (int j = 0; j < lenth; j++) {
                indexFile >> center[i].vectors[j];
            }
        }
        // circle
        int circleSize;
        indexFile >> circleSize;
        circle.resize(circleSize);
        for (int i = 0; i < circleSize; i++) {
            int lenth;
            indexFile >> lenth;
            circle[i].resize(lenth);
            for (int j = 0; j < lenth; j++) {
                indexFile >> circle[i][j];
            }
        }
        // sparse
        int sparseSize;
        indexFile >> sparseSize;
        sparse.resize(sparseSize);
        for (int i = 0; i < sparseSize; i++) {
            indexFile >> sparse[i];
        }
        indexFile.close();

        auto centerName = bothName + ".center";
        auto sparseName = bothName + ".sparse";
        auto circleName = bothName + ".circle_";
        if (DatabaseSelect >= 10) {
            centerIndex = new hnswlib::HierarchicalNSW<float>(centerSpaceIP, centerName.c_str());
        } else {
            centerIndex = new hnswlib::HierarchicalNSW<float>(centerSpaceL2, centerName.c_str());
        }
        if (DatabaseSelect >= 10) {
            sparseIndex = new hnswlib::HierarchicalNSW<float>(sparseSpaceIP, sparseName.c_str());
        } else {
            sparseIndex = new hnswlib::HierarchicalNSW<float>(sparseSpaceL2, sparseName.c_str());
        }
        for (int i = 0; i < circleSize; i++) {
            auto thisName = circleName + std::to_string(i) ;
            if (DatabaseSelect >= 10) {
                circleIndex[i] = new hnswlib::HierarchicalNSW<float>(&circleSpaceIP[i], thisName.c_str());
            } else {
                circleIndex[i] = new hnswlib::HierarchicalNSW<float>(&circleSpaceL2[i], thisName.c_str());
            }
        }
    }
    void searchIndex() {
        // Query the elements for themselves and measure recall
        float correct = 0;
        double allTime = 0;
        for (int i = 0; i < dataSet->queryData.size(); i++) {
            std::vector<float> temp;
            if (DatabaseSelect >= 10) {
                temp = norm_vector(dataSet->queryData[i].vectors);
            } else {
                temp = dataSet->queryData[i].vectors;
            }
            std::vector<std::pair<float, int>> result;
            auto start = std::chrono::steady_clock::now();
            auto resultSparse = sparseIndex->searchKnn(temp.data(), K);
            auto end = std::chrono::steady_clock::now();
            allTime += (end - start)/ 1us;
            while (!resultSparse.empty()) {
                result.push_back(resultSparse.top());
                resultSparse.pop();
            }
            start = std::chrono::steady_clock::now();
            auto resultCenter = centerIndex->searchKnn(temp.data(), search_center);
            end = std::chrono::steady_clock::now();
            allTime += (end - start)/ 1us;
            std::vector<int> circleWait;

            while (!resultCenter.empty()) {
                int ans = resultCenter.top().second;
                resultCenter.pop();
                circleWait.push_back(ans);
            }
            start = std::chrono::steady_clock::now();
            #pragma omp parallel for
            for (auto & ans : circleWait) {
                auto circleResult = circleIndex[ans]->searchKnn(temp.data(), K);
                while (!circleResult.empty()) {
                    #pragma omp critical
                    {
                        result.push_back(circleResult.top());
                    }
                    circleResult.pop();
                }
            }
            end = std::chrono::steady_clock::now();
            std::sort(result.begin(), result.end(), [](const std::pair<float, int> &a, const std::pair<float, int> &b) {
                return a.first < b.first;
            });
            // 计算正确率
            for (int j = 0; j < K; j++) {
                int ans = result[j].second;
                for (auto & ansItem : dataSet->ansData[i].vectors) {
                    if (ans == ansItem) {
                        correct++;
                        break;
                    }
                }
            }
        }
        float recall = correct / (dataSet->queryData.size() * K);
        logFile << "Recall: " << recall << std::endl;
        logFile << "avgTime: " << allTime / dataSet->queryData.size() << "us" << std::endl;
    }
};

#endif