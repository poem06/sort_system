import random

def generate_mixed_data(count=50, min_val=-100, max_val=100):
    # 生成数据
    data = [random.randint(min_val, max_val) for _ in range(count)]
    
    # 转换为字符串（无空格）
    data_str = ",".join(map(str, data))
    
    print(data_str)
    
    # 保存文件
    with open("sort_data_mixed.txt", "w") as f:
        f.write(data_str)

if __name__ == "__main__":
    generate_mixed_data()