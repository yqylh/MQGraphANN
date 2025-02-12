#include "lib/DataSet.cpp"
#include "lib/Config.cpp"
#include "lib/CircleIndex.cpp"

int main(int argc, char **argv ){
    std::ios::sync_with_stdio(false);
    std::cin.tie(0);
    
    M = atoi(argv[1]);
    ef = atoi(argv[2]);
    cluster_num = atoi(argv[3]);
    m_circle = atoi(argv[4]);
    m_sparse = atoi(argv[5]);
    CircleIndex index(baseFileName, M, ef, cluster_num, m_circle, m_sparse);

    return 0;
}
