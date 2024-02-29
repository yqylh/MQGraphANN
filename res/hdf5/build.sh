g++ importH5.cpp -o importH5 -g \
    -std=c++11 \
    -I /usr/include/hdf5/serial \
    -I ./HighFive/include/ \
    -lhdf5_cpp -lhdf5 -L /usr/lib/x86_64-linux-gnu/hdf5/serial

./importH5
# -I 头文件路径
# -L 静态库文件路径
# -l 静态库文件名