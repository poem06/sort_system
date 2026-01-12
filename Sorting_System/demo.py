import random

def generate_test_data(count=50, min_val=0, max_val=100, filename="sort_data.txt"):
    # 1. 生成随机数据
    data = [random.randint(min_val, max_val) for _ in range(count)]
    
    # 2. 转换为字符串并用逗号连接（无空格）
    data_str = ",".join(map(str, data))
    
    # 3. 打印到控制台供复制
    print(f"--- 生成的 {count} 个数据 ---")
    print(data_str)
    
    # 4. (可选) 写入文件，方便 C 语言 fscanf 读取
    with open(filename, "w") as f:
        f.write(data_str)
    print(f"\n[完成] 数据已保存至 {filename}")

# 执行生成
if __name__ == "__main__":
    generate_test_data(50)