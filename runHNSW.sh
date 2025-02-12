dataset=(1 7 8 9 10 11 12 13)
edge=(8 16 24 32 48 64 96 128)
ef=(200)
for datasetS in ${dataset[*]}
do
    g++ hnsw.cpp -o hnsw -g -std=c++17 -O3 -DDatabaseSelect=$datasetS  -fopenmp -pthread -w \
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