dataset=$1
edge=(8 16 24 32 40 48 56 64 72 80 88 96 104 112 120 128)
ef=(1 5 10 20 40 60 80 100 120 140 160 180 200 250 300 350 400 450 500)
CLUSTER_NUM=(100 200 300 400 500 600 700 800 900 1000)
CLUSTER_CORE=(2 3 4 5 6 8 10 12 14 16 18 20 25 30 35 40 45 50)

for edgeS in ${edge[*]}
do
    for efS in ${ef[*]}
    do
        g++ path.cpp -o path$dataset -g -std=c++17 -O3 -DDatabaseSelect=$dataset -DTEST -fopenmp -pthread -w \
            -I /usr/include/hdf5/serial \
            -I ./res/hdf5/HighFive/include/ \
            -lhdf5_cpp -lhdf5 -L /usr/lib/x86_64-linux-gnu/hdf5/serial
        ./path$dataset $edgeS $efS

        g++ path.cpp -o path$dataset -g -std=c++17 -O3 -DDatabaseSelect=$dataset -DTEST -DZERO -fopenmp -pthread -w \
            -I /usr/include/hdf5/serial \
            -I ./res/hdf5/HighFive/include/ \
            -lhdf5_cpp -lhdf5 -L /usr/lib/x86_64-linux-gnu/hdf5/serial
        ./path$dataset $edgeS $efS

        g++ path.cpp -o path$dataset -g -std=c++17 -O3 -DDatabaseSelect=$dataset -DTEST -DZERO -DCLUSTER -fopenmp -pthread -w \
            -I /usr/include/hdf5/serial \
            -I ./res/hdf5/HighFive/include/ \
            -lhdf5_cpp -lhdf5 -L /usr/lib/x86_64-linux-gnu/hdf5/serial
        for CLUSTER_CORES in ${CLUSTER_CORE[*]}
        do
            for CLUSTER_NUMS in ${CLUSTER_NUM[*]}
            do
            {
                ./path$dataset $edgeS $efS $CLUSTER_NUMS $CLUSTER_CORES
            }&
            done
            wait
        done
    done
done
