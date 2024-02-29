dataset=(1 7 8 9 10 11 12 13)
CLUSTER_NUM=(100 200 300 400 500 600 700 800 900 1000)
for datasetS in ${dataset[*]}
do
    g++ kmeansTime.cpp -o times -g -std=c++17 -O3 -DDatabaseSelect=$datasetS -DTEST -DZERO -DCLUSTER -fopenmp -pthread -w \
        -I /usr/include/hdf5/serial \
        -I ./res/hdf5/HighFive/include/ \
        -lhdf5_cpp -lhdf5 -L /usr/lib/x86_64-linux-gnu/hdf5/serial
        for CLUSTER_NUMS in ${CLUSTER_NUM[*]}
        do
        {
            ./times 8 1 $CLUSTER_NUMS 2
        }
        done
done