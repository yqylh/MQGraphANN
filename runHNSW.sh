dataset=(1 7 8 9 10 11 12 13)
edge=(8 16 24 32 40 48 56 64 72 80 88 96 104 112 120 128)
ef=(1 5 10 20 40 60 80 100 120 140 160 180 200 250 300 350 400 450 500)
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