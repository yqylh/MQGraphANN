# dataset=(1 7 8 10 11 12)
datasetS=8
g++ circle.cpp -o main -g -std=c++17 -O3 -DDatabaseSelect=$datasetS  -fopenmp -pthread -w \
    -I /usr/include/hdf5/serial \
    -I ./res/hdf5/HighFive/include/ \
    -lhdf5_cpp -lhdf5 -L /usr/lib/x86_64-linux-gnu/hdf5/serial

./main 200 48 2 10
rm main