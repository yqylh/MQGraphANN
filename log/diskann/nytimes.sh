dataset=(nytimes)

diskann=~/yqy/DiskANN/build/apps/

for i in ${dataset[@]}; do
    # 转换数据格式 finished
    # {
    #     $diskann/utils/fvecs_to_bin float $i/base.fvecs $i/base.fbin
    #     $diskann/utils/fvecs_to_bin float $i/query.fvecs $i/query.fbin
    # }
    # 创建groundtruth finished
    # {
    #     $diskann/utils/compute_groundtruth --data_type float --dist_fn mips --base_file ./$i/base.fbin --query_file ./$i/query.fbin --gt_file ./$i/gt100 --K 100
    # }
    # 构建内存索引 finished
    # {
    #     # R 也就是 M 邻居数
    #     # L 也就是 ef 队列大小
    #     # --α（默认值为 1.2）： 一个介于 1.0 和 1.5 之间的浮点数值，它决定了图形的直径，大约为基数 alpha 的 log n。典型值在 1 到 1.5 之间。1 将产生最稀疏的图形，1.5 将产生更密集的图形。
    #     # T (--num_threads)（默认为 get_omp_num_procs()）：建立索引过程中使用的线程数。由于代码是高度并行的，索引时间几乎与线程数呈线性关系（取决于机器上可用的内核和 DRAM 带宽）。
    #     R_all=(8 16 24 32 48 64 96 128)
    #     L=200
    #     alpha=1.2
    #     num_threads=8
    #     for R in ${R_all[@]}; do
    #         echo "====== dataset: $i, R: $R, L: $L, alpha: $alpha, num_threads: $num_threads ======"
    #         indexName="${i}/index_R${R}_L${L}_A${alpha}"
    #         $diskann/build_memory_index  --data_type float --dist_fn mips --data_path ./$i/base.fbin --index_path_prefix $indexName -R $R -L $L --alpha $alpha --num_threads $num_threads
    #     done
    # }
    # 查询
    {
        # ../search_memory_index  --data_type float --dist_fn l2 --index_path_prefix ./index_base_learn_R32_L50_A1.2 --query_file ./sift-query.fbin  --gt_file ./sift_query_learn_gt100 -K 10 -L 10 20 30 40 50 100 --result_path ./sift-res
        R_all=(8 16 24 32 48 64 96 128)
        L=200
        alpha=1.2
        num_threads=8
        for R in ${R_all[@]}; do
            echo "====== dataset: $i, R: $R, L: $L, alpha: $alpha, num_threads: $num_threads ======"
            indexName="${i}/index_R${R}_L${L}_A${alpha}"
            $diskann/search_memory_index  --data_type float --dist_fn mips --index_path_prefix $indexName --query_file ./$i/query.fbin --gt_file ./$i/gt100 -K 100 -L $L --result_path ./$i/
        done
    }
done