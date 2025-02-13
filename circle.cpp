#include "lib/DataSet.cpp"
#include "lib/Config.cpp"
#include "lib/CircleIndex.cpp"

extern std::string baseFileName;

int main(int argc, char **argv ){
    std::ios::sync_with_stdio(false);
    std::cin.tie(0);
    
    ef = atoi(argv[1]);
    m_circle = atoi(argv[2]);
    m_sparse = atoi(argv[3]);
    search_center = atoi(argv[4]);
    CircleIndex index(baseFileName, ef, m_circle, m_sparse, search_center);
    index.buildIndex();
    // index.loadIndex();
    index.searchIndex();
    return 0;
}
