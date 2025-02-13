dataset=(1 7 8 10 11 12)
edge=(8 16 24 32 48 64 96 128)
ef=200

# 对每个 dataset 开启一个后台子进程
for datasetS in "${dataset[@]}"; do
(
    # 定义该 dataset 对应的二进制文件名
    binary_name="hnsw_${datasetS}"

    # 编译，注意使用 -DDatabaseSelect 宏传入 datasetS
    g++ hnsw.cpp -o "$binary_name" -g -std=c++17 -O3 -DDatabaseSelect=$datasetS -fopenmp -pthread -w \
        -I /usr/include/hdf5/serial \
        -I ./res/hdf5/HighFive/include/ \
        -lhdf5_cpp -lhdf5 -L /usr/lib/x86_64-linux-gnu/hdf5/serial

    # 对该 dataset 下的每个 edge 同样开启后台任务
    for edgeS in "${edge[@]}"; do
    {
        # 执行程序，传入 edgeS 和 ef 两个参数
        ./"$binary_name" "$edgeS" "$ef"
    } &
    done
    # 等待当前 dataset 下所有 edge 任务完成
    wait
) &
done
# 等待所有 dataset 的后台任务完成
wait