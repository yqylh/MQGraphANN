# dataset=(1 7 8 9 10 11 12 13)
datasetS=11
g++ test-avg-knn.cpp -o main -g -std=c++17 -O3 -DDatabaseSelect=$datasetS -DTEST -DZERO -DCLUSTER -fopenmp -pthread -w \
    -I /usr/include/hdf5/serial \
    -I ./res/hdf5/HighFive/include/ \
    -lhdf5_cpp -lhdf5 -L /usr/lib/x86_64-linux-gnu/hdf5/serial

./main 8 1 1 2
rm main