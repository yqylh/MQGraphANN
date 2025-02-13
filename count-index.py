import os
from collections import defaultdict

def convert_size(size_bytes):
    """将字节数转换为易读的格式"""
    if size_bytes == 0:
        return "0B"
    units = ('B', 'KB', 'MB', 'GB', 'TB')
    i = 0
    while size_bytes >= 1024 and i < len(units)-1:
        size_bytes /= 1024
        i += 1
    return f"{size_bytes:.2f} {units[i]}"

def analyze_files(folder_path):
    """分析文件并统计信息"""
    stats = defaultdict(lambda: {
        'total_files': 0,
        'total_size': 0,
        'circle': {'count': 0, 'size': 0},
        'index': {'count': 0, 'size': 0},
        'sparse': {'count': 0, 'size': 0},
        'center': {'count': 0, 'size': 0},
    })

    for entry in os.scandir(folder_path):
        if not entry.is_file():
            continue
        
        filename = entry.name
        
        # 解析文件名各部分
        try:
            # 分割datasetid部分
            base_parts = filename.split('_ef_200_m_circle_')
            if len(base_parts) != 2:
                continue
            dataset_id, remainder = base_parts

            # 分割p1和后缀部分
            suffix_parts = remainder.split('_m_sparse_2.')
            if len(suffix_parts) != 2:
                continue
            p1, file_type = suffix_parts

            # 确定文件类型
            if file_type.startswith('circle_'):
                category = 'circle'
            elif file_type in ['index', 'sparse', 'center']:
                category = file_type
            else:
                continue

        except Exception as e:
            continue

        # 获取文件大小
        try:
            file_size = entry.stat().st_size
        except OSError:
            continue

        # 更新统计信息
        key = (dataset_id, p1)
        stats[key]['total_files'] += 1
        stats[key]['total_size'] += file_size
        stats[key][category]['count'] += 1
        stats[key][category]['size'] += file_size

    # 打印统计结果
    for key in stats:
        dataset_id, p1 = key
        data = stats[key]
        print(f"\nDataset ID: {dataset_id} | P1: {p1}")
        print(f"Total Files: {data['total_files']}")
        print(f"Total Size: {convert_size(data['total_size'])}")
        print("Category Breakdown:")
        print(f"  Circle: {data['circle']['count']} files ({convert_size(data['circle']['size'])})")
        print(f"  Index:  {data['index']['count']} files ({convert_size(data['index']['size'])})")
        print(f"  Sparse: {data['sparse']['count']} files ({convert_size(data['sparse']['size'])})")
        print(f"  Center: {data['center']['count']} files ({convert_size(data['center']['size'])})")
        print("-" * 60)

if __name__ == "__main__":
    import sys
    if len(sys.argv) != 2:
        print("Usage: python file_stats.py <folder_path>")
        sys.exit(1)
    
    target_folder = sys.argv[1]
    if not os.path.isdir(target_folder):
        print(f"Error: {target_folder} is not a valid directory")
        sys.exit(1)
    
    analyze_files(target_folder)