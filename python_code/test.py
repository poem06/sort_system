import tkinter as tk
from tkinter import ttk, messagebox
import ctypes
import os
import random
import time
import threading


# ==========================================
# 1. C语言 DLL 接口定义
# ==========================================
class SortStats(ctypes.Structure):
    _fields_ = [
        ("comparisons", ctypes.c_longlong),
        ("moves", ctypes.c_longlong),
        ("time_ms", ctypes.c_double)
    ]


# 加载 DLL
try:
    # 尝试加载当前目录下的 DLL
    dll_path = os.path.join(os.path.dirname(__file__), "Sorting_ASystem.dll")
    # 如果是 VS 生成的 Debug 目录，可能需要修改路径，例如:
    # dll_path = r"x64\Debug\Sorting_ASystem.dll"

    clib = ctypes.CDLL(dll_path)
except OSError:
    print("警告: 未找到 Sorting_ASystem.dll，C语言性能模式将不可用。")
    clib = None

# 配置 C 函数签名
if clib:
    # 通用参数配置
    CMP_FUNC = ctypes.CFUNCTYPE(SortStats, ctypes.POINTER(ctypes.c_int), ctypes.c_int)

    clib.bubble_sort.restype = SortStats
    clib.insertion_sort.restype = SortStats
    clib.selection_sort.restype = SortStats
    clib.shell_sort.restype = SortStats
    clib.quick_sort.restype = SortStats
    clib.merge_sort.restype = SortStats
    clib.heap_sort.restype = SortStats

    clib.generate_data.argtypes = [ctypes.POINTER(ctypes.c_int), ctypes.c_int, ctypes.c_int]


# ==========================================
# 2. 排序算法可视化逻辑 (Python生成器)
# ==========================================
# 为了动画效果，这里需要在 Python 侧“模拟”一遍排序过程
# yield 关键字用于暂停并告诉 UI "该画图了"

def py_bubble_sort(data):
    n = len(data)
    for i in range(n - 1):
        for j in range(n - i - 1):
            yield data, [j, j + 1], -1  # 比较状态: 红色
            if data[j] > data[j + 1]:
                data[j], data[j + 1] = data[j + 1], data[j]
                yield data, [j, j + 1], 1  # 交换状态: 绿色
    yield data, [], 0  # 完成


def py_insertion_sort(data):
    n = len(data)
    for i in range(1, n):
        key = data[i]
        j = i - 1
        while j >= 0 and data[j] > key:
            yield data, [j, j + 1], -1
            data[j + 1] = data[j]
            yield data, [j, j + 1], 1
            j -= 1
        data[j + 1] = key
        yield data, [j + 1], 1
    yield data, [], 0


def py_selection_sort(data):
    n = len(data)
    for i in range(n - 1):
        min_idx = i
        for j in range(i + 1, n):
            yield data, [j, min_idx], -1
            if data[j] < data[min_idx]:
                min_idx = j
        if min_idx != i:
            data[i], data[min_idx] = data[min_idx], data[i]
            yield data, [i, min_idx], 1
    yield data, [], 0


def py_shell_sort(data):
    n = len(data)
    gap = n // 2
    while gap > 0:
        for i in range(gap, n):
            temp = data[i]
            j = i
            while j >= gap and data[j - gap] > temp:
                yield data, [j, j - gap], -1
                data[j] = data[j - gap]
                yield data, [j, j - gap], 1
                j -= gap
            data[j] = temp
            yield data, [j], 1
        gap //= 2
    yield data, [], 0


def py_quick_sort(data, low, high):
    if low < high:
        # Partition logic inside
        pivot = data[high]
        i = low - 1
        for j in range(low, high):
            yield data, [j, high], -1
            if data[j] < pivot:
                i += 1
                data[i], data[j] = data[j], data[i]
                yield data, [i, j], 1
        data[i + 1], data[high] = data[high], data[i + 1]
        yield data, [i + 1, high], 1
        pi = i + 1

        yield from py_quick_sort(data, low, pi - 1)
        yield from py_quick_sort(data, pi + 1, high)


def py_quick_sort_wrapper(data):
    yield from py_quick_sort(data, 0, len(data) - 1)
    yield data, [], 0


def py_merge_sort(data, left, right):
    if left >= right:
        return

    mid = (left + right) // 2
    yield from py_merge_sort(data, left, mid)
    yield from py_merge_sort(data, mid + 1, right)

    # Merge过程的可视化
    temp = []
    i = left
    j = mid + 1

    while i <= mid and j <= right:
        yield data, [i, j], -1  # 比较颜色
        if data[i] <= data[j]:
            temp.append(data[i])
            i += 1
        else:
            temp.append(data[j])
            j += 1

    while i <= mid:
        temp.append(data[i])
        i += 1
    while j <= right:
        temp.append(data[j])
        j += 1

    # 将排序好的 temp 放回 data，并展示动画
    for k in range(len(temp)):
        idx = left + k
        data[idx] = temp[k]
        yield data, [idx], 1  # 写入颜色

def py_merge_sort_wrapper(data):
    yield from py_merge_sort(data, 0, len(data) - 1)
    yield data, [], 0


def py_heapify(data, n, i):
    largest = i
    l = 2 * i + 1
    r = 2 * i + 2

    if l < n:
        yield data, [l, largest], -1
        if data[l] > data[largest]:
            largest = l

    if r < n:
        yield data, [r, largest], -1
        if data[r] > data[largest]:
            largest = r

    if largest != i:
        data[i], data[largest] = data[largest], data[i]
        yield data, [i, largest], 1
        yield from py_heapify(data, n, largest)

def py_heap_sort(data):
    n = len(data)
    # 建堆
    for i in range(n // 2 - 1, -1, -1):
        yield from py_heapify(data, n, i)

    # 排序
    for i in range(n - 1, 0, -1):
        data[i], data[0] = data[0], data[i]
        yield data, [i, 0], 1
        yield from py_heapify(data, i, 0)
    yield data, [], 0


# ==========================================
# 3. 可视化系统 GUI 主程序
# ==========================================
class VisualSortApp:
    def __init__(self, root):
        self.root = root
        self.root.title("排序算法可视化系统 - C语言后端内核")
        self.root.geometry("1000x700")
        self.root.configure(bg="#2b2b2b")  # 深色背景

        # 状态变量
        self.data = []
        self.original_data = []  # 备份用于重置
        self.is_sorting = False
        self.is_paused = False
        self.speed_ms = 50
        self.algo_generator = None
        self.c_stats = None  # 存储C语言返回的性能数据

        # 算法映射
        self.algos = {
            "冒泡排序": (clib.bubble_sort if clib else None, py_bubble_sort),
            "插入排序": (clib.insertion_sort if clib else None, py_insertion_sort),
            "选择排序": (clib.selection_sort if clib else None, py_selection_sort),
            "希尔排序": (clib.shell_sort if clib else None, py_shell_sort),
            "快速排序": (clib.quick_sort if clib else None, py_quick_sort_wrapper),
            "归并排序": (clib.merge_sort if clib else None, py_merge_sort_wrapper),  # 新增
            "堆排序": (clib.heap_sort if clib else None, py_heap_sort),  # 新增
        }

        self._setup_ui()
        self.generate_data()

    def _setup_ui(self):
        # --- 1. 顶部信息栏 ---
        info_frame = tk.Frame(self.root, bg="#1e1e1e", height=60)
        info_frame.pack(fill=tk.X, side=tk.TOP)

        self.lbl_algo_name = tk.Label(info_frame, text="当前算法: 未选择", font=("微软雅黑", 12, "bold"), bg="#1e1e1e",
                                      fg="#00ffcc")
        self.lbl_algo_name.pack(side=tk.LEFT, padx=20, pady=10)

        # 性能数据展示区
        stats_frame = tk.Frame(info_frame, bg="#1e1e1e")
        stats_frame.pack(side=tk.RIGHT, padx=20)

        self.lbl_stats = tk.Label(stats_frame, text="数据量: 0   比较次数: 0   移动次数: 0   耗时: 0.00ms",
                                  font=("Consolas", 10), bg="#1e1e1e", fg="white")
        self.lbl_stats.pack()

        # --- 2. 中间可视化画布 ---
        self.canvas = tk.Canvas(self.root, bg="#232323", highlightthickness=0)
        self.canvas.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)

        # --- 3. 底部控制栏 ---
        ctrl_frame = tk.Frame(self.root, bg="#333333", height=80)
        ctrl_frame.pack(fill=tk.X, side=tk.BOTTOM)

        # 左侧数据控制
        tk.Button(ctrl_frame, text="生成数据", command=self.generate_data, bg="#444", fg="white", width=10).pack(
            side=tk.LEFT, padx=10, pady=15)
        self.scale_size = tk.Scale(ctrl_frame, from_=10, to=200, orient=tk.HORIZONTAL, label="数据量", bg="#333",
                                   fg="white", command=self.on_size_change)
        self.scale_size.set(50)
        self.scale_size.pack(side=tk.LEFT, padx=10)

        # 中间播放控制
        self.algo_var = tk.StringVar(value="冒泡排序")
        ttk.Combobox(ctrl_frame, textvariable=self.algo_var, values=list(self.algos.keys()), state="readonly",
                     width=15).pack(side=tk.LEFT, padx=20)

        # 速度控制
        self.scale_speed = tk.Scale(ctrl_frame, from_=1, to=100, orient=tk.HORIZONTAL, label="演示速度(快-慢)",
                                    bg="#333", fg="white")
        self.scale_speed.set(20)  # 默认较快
        self.scale_speed.pack(side=tk.LEFT, padx=10)

        # 核心按钮 (使用 Unicode 符号代替图片，防止路径报错，你可以替换为 Image)
        btn_style = {"bg": "#007acc", "fg": "white", "font": ("Arial", 12, "bold"), "width": 4}
        tk.Button(ctrl_frame, text="▶", command=self.start_sort, **btn_style).pack(side=tk.LEFT, padx=5)  # 开始
        tk.Button(ctrl_frame, text="⏸", command=self.pause_sort, **btn_style).pack(side=tk.LEFT, padx=5)  # 暂停
        tk.Button(ctrl_frame, text="⚡", command=self.run_c_benchmark, bg="#e0aa00", fg="white", font=("Arial", 12),
                  width=4).pack(side=tk.LEFT, padx=5)  # 极速模式

        # 状态栏
        self.status_bar = tk.Label(self.root, text="就绪 - 点击 ▶ 开始可视化，点击 ⚡ 进行C语言极速测速", bg="#2b2b2b",
                                   fg="#888", anchor="w")
        self.status_bar.pack(fill=tk.X, side=tk.BOTTOM)

    def on_size_change(self, val):
        self.generate_data()

    def generate_data(self):
        if self.is_sorting: return
        n = self.scale_size.get()
        # 随机生成高度 (10 到 画布高度-20)
        h = self.canvas.winfo_height() if self.canvas.winfo_height() > 10 else 500
        self.data = [random.randint(10, h - 50) for _ in range(n)]
        self.original_data = self.data[:]
        self.draw_bars(self.data, [], -1)
        self.lbl_stats.config(text=f"数据量: {n}   比较次数: 0   移动次数: 0   耗时: 0.00ms")

    def draw_bars(self, data, highlights, state):
        self.canvas.delete("all")
        w = self.canvas.winfo_width()
        h = self.canvas.winfo_height()
        n = len(data)
        bar_w = w / n

        for i, val in enumerate(data):
            x0 = i * bar_w
            y0 = h - val
            x1 = x0 + bar_w - 1
            y1 = h

            # 颜色逻辑
            color = "#4a90e2"  # 默认蓝
            if i in highlights:
                if state == 1:
                    color = "#50e3c2"  # 交换: 绿
                elif state == -1:
                    color = "#e35050"  # 比较: 红

            self.canvas.create_rectangle(x0, y0, x1, y1, fill=color, outline="")

    def start_sort(self):
        if self.is_sorting: return
        self.is_sorting = True
        self.is_paused = False

        algo_name = self.algo_var.get()
        self.lbl_algo_name.config(text=f"当前算法: {algo_name}")

        # 重置数据
        self.data = self.original_data[:]

        # 获取 Python 生成器
        _, py_func = self.algos[algo_name]
        self.algo_generator = py_func(self.data)

        # 启动动画循环
        self.animate()

    def pause_sort(self):
        self.is_paused = not self.is_paused

    def animate(self):
        if not self.is_sorting: return
        if self.is_paused:
            self.root.after(100, self.animate)
            return

        try:
            # 获取下一步状态
            data_snapshot, highlights, state = next(self.algo_generator)
            self.draw_bars(data_snapshot, highlights, state)

            # 速度控制
            speed = self.scale_speed.get()
            self.root.after(speed, self.animate)

        except StopIteration:
            self.is_sorting = False
            self.draw_bars(self.data, [], 0)
            self.status_bar.config(text="排序完成！(可视化仅为演示，真实性能请点击⚡)")
            # 自动运行一次 C 语言基准测试来显示真实数据
            # self.run_c_benchmark()

    def run_c_benchmark(self):
        """调用 C 语言 DLL 进行极速排序和测速"""
        if not clib:
            messagebox.showerror("错误", "未加载 DLL，无法运行 C 语言后端")
            return

        algo_name = self.algo_var.get()
        c_func, _ = self.algos[algo_name]

        if not c_func:
            return

        # 准备数据 (C需要 int 数组)
        n = len(self.original_data)
        ArrayType = ctypes.c_int * n
        c_arr = ArrayType(*self.original_data)

        # 调用 C 函数
        self.status_bar.config(text=f"正在调用 C 语言内核执行 {algo_name}...")
        self.root.update()

        stats = c_func(c_arr, n)  # 这一步瞬间完成

        # 更新 UI
        self.lbl_stats.config(
            text=f"数据量: {n}   比较次数: {stats.comparisons}   移动次数: {stats.moves}   耗时: {stats.time_ms:.4f}ms")
        self.status_bar.config(text=f"C语言内核执行完毕。耗时 {stats.time_ms:.4f} 毫秒")

        # 将排序后的结果画出来
        sorted_data = list(c_arr)
        self.draw_bars(sorted_data, [], 0)
        self.data = sorted_data
        self.is_sorting = False


if __name__ == "__main__":
    root = tk.Tk()
    app = VisualSortApp(root)
    root.mainloop()
