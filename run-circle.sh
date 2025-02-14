dataset=(1 7 8 10 11 12)
edge=(8 16 24 32 48 64 96 128)
ef=200

for datasetS in "${dataset[@]}"; do
{
    g++ circle.cpp -o circle -g -std=c++17 -O3 -DDatabaseSelect=$datasetS -fopenmp -pthread -w \
        -I /usr/include/hdf5/serial \
        -I ./res/hdf5/HighFive/include/ \
        -lhdf5_cpp -lhdf5 -L /usr/lib/x86_64-linux-gnu/hdf5/serial

    for edgeS in "${edge[@]}"; do
    {
        ./circle 200 $edgeS 16 25
        echo "dataset: $datasetS, edge: $edgeS"
    }
    done
    rm circle
}
done