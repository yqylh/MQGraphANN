dataset=(1 7 9 10 11 12)
edge=(8 16 24 32 48 64 96 128)
ef=200
for datasetS in ${dataset[*]}
do
    g++ hnsw.cpp -o hnsw -g -std=c++17 -O3 -DDatabaseSelect=$datasetS  -fopenmp -pthread -w \
        -I /usr/include/hdf5/serial \
        -I ./res/hdf5/HighFive/include/ \
        -lhdf5_cpp -lhdf5 -L /usr/lib/x86_64-linux-gnu/hdf5/serial
    for edgeS in ${edge[*]}
    do
    {
        # 记录开始时间（秒.纳秒）
        start=$(date +%s.%N)
        # 注意：这里第二个参数用 ef 而不是 efS
        ./hnsw $edgeS $ef
        # 记录结束时间
        end=$(date +%s.%N)
        # 计算运行时长
        runtime=$(echo "$end - $start" | bc)
        # 保存结果到文件中
        echo "Dataset: $datasetS, m: $edgeS, runtime: ${runtime} seconds" >> "$result_file"
    }&
    done
    wait
done