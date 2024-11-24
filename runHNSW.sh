dataset=(1)
edge=(8)
ef=(1)
for datasetS in ${dataset[*]}
do
    g++ hnsw.cpp -o hnsw -g -std=c++17 -O3 -DDatabaseSelect=$datasetS -DTEST -fopenmp -pthread -w \
        -I /usr/include/hdf5/serial \
        -I ./res/hdf5/HighFive/include/ \
        -lhdf5_cpp -lhdf5 -L /usr/lib/x86_64-linux-gnu/hdf5/serial
    for efS in ${ef[*]}
    do
        for edgeS in ${edge[*]}
        do
        {
            ./hnsw $edgeS $efS
        }&
        done
        wait
    done
done