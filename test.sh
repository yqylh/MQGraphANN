g++ kmeansTime.cpp -o times -g -std=c++17 -O3 -DDatabaseSelect=1 -DTEST -DZERO -DCLUSTER -fopenmp -pthread -w \
    -I /usr/include/hdf5/serial \
    -I ./res/hdf5/HighFive/include/ \
    -lhdf5_cpp -lhdf5 -L /usr/lib/x86_64-linux-gnu/hdf5/serial