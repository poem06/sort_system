import tkinter as tk
from tkinter import ttk, messagebox, simpledialog
import ctypes
import os
import random
import time
import threading

# ==========================================
# 0. 算法理论数据 (新增：用于右侧面板显示)
# ==========================================
ALGO_INFO = {
    "冒泡排序": {
        "time": "O(n²)",
        "space": "O(1)",
        "stability": "稳定",
        "reason": "只有相邻且左大于右时才交换，相等元素不交换，故保持相对顺序。"
    },
    "插入排序": {
        "time": "O(n²)",
        "space": "O(1)",
        "stability": "稳定",
        "reason": "从后向前扫描，遇到相等元素插在其后面，不改变相对顺序。"
    },
    "选择排序": {
        "time": "O(n²)",
        "space": "O(1)",
        "stability": "不稳定",
        "reason": "交换可能将前面的元素跨越交换到后面，破坏相对顺序 (如 [2a, 2b, 1] -> [1, 2b, 2a])。"
    },
    "希尔排序": {
        "time": "O(n log n) ~ O(n²)",
        "space": "O(1)",
        "stability": "不稳定",
        "reason": "分组进行插入排序，相同的数可能被分在不同组，跳跃式移动破坏相对顺序。"
    },
    "快速排序": {
        "time": "O(n log n)",
        "space": "O(log n)",
        "stability": "不稳定",
        "reason": "分区过程中的交换涉及远距离非相邻元素，无法保证相对顺序。"
    },
    "归并排序": {
        "time": "O(n log n)",
        "space": "O(n)",
        "stability": "稳定",
        "reason": "合并两个有序子数组时，若左右元素相等，优先取左侧元素，保持相对顺序。"
    },
    "堆排序": {
        "time": "O(n log n)",
        "space": "O(1)",
        "stability": "不稳定",
        "reason": "建堆和调整堆的过程中，父子节点的交换会破坏相同元素的相对位置。"
    }
}


# ==========================================
# 1. C语言 DLL 接口定义 (保持不变)
# ==========================================
class SortStats(ctypes.Structure):
    _fields_ = [
        ("comparisons", ctypes.c_longlong),
        ("moves", ctypes.c_longlong),
        ("time_ms", ctypes.c_double)
    ]


# 加载 DLL
try:
    dll_path = os.path.join(os.path.dirname(__file__), "Sorting_ASystem.dll")
    clib = ctypes.CDLL(dll_path)
except OSError:
    print("警告: 未找到 Sorting_ASystem.dll，C语言性能模式将不可用。")
    clib = None

# 配置 C 函数签名
if clib:
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
# 2. 排序算法可视化逻辑 (Python生成器 - 保持不变)
# ==========================================
def py_bubble_sort(data):
    n = len(data)
    for i in range(n - 1):
        for j in range(n - i - 1):
            yield data, [j, j + 1], -1
            if data[j] > data[j + 1]:
                data[j], data[j + 1] = data[j + 1], data[j]
                yield data, [j, j + 1], 1
    yield data, [], 0


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
    if left >= right: return
    mid = (left + right) // 2
    yield from py_merge_sort(data, left, mid)
    yield from py_merge_sort(data, mid + 1, right)
    temp = []
    i = left
    j = mid + 1
    while i <= mid and j <= right:
        yield data, [i, j], -1
        if data[i] <= data[j]:
            temp.append(data[i]);
            i += 1
        else:
            temp.append(data[j]);
            j += 1
    while i <= mid: temp.append(data[i]); i += 1
    while j <= right: temp.append(data[j]); j += 1
    for k in range(len(temp)):
        idx = left + k
        data[idx] = temp[k]
        yield data, [idx], 1


def py_merge_sort_wrapper(data):
    yield from py_merge_sort(data, 0, len(data) - 1)
    yield data, [], 0


def py_heapify(data, n, i):
    largest = i
    l = 2 * i + 1
    r = 2 * i + 2
    if l < n:
        yield data, [l, largest], -1
        if data[l] > data[largest]: largest = l
    if r < n:
        yield data, [r, largest], -1
        if data[r] > data[largest]: largest = r
    if largest != i:
        data[i], data[largest] = data[largest], data[i]
        yield data, [i, largest], 1
        yield from py_heapify(data, n, largest)


def py_heap_sort(data):
    n = len(data)
    for i in range(n // 2 - 1, -1, -1):
        yield from py_heapify(data, n, i)
    for i in range(n - 1, 0, -1):
        data[i], data[0] = data[0], data[i]
        yield data, [i, 0], 1
        yield from py_heapify(data, i, 0)
    yield data, [], 0


# ==========================================
# 3. 可视化系统 GUI 主程序 (界面升级)
# ==========================================
class VisualSortApp:
    def __init__(self, root):
        self.root = root
        self.root.title("排序算法可视化系统 - C语言后端内核")
        self.root.geometry("1100x750")  # 稍微加宽以容纳右侧面板
        self.root.configure(bg="#2b2b2b")

        self.data = []
        self.original_data = []
        self.is_sorting = False
        self.is_paused = False
        self.speed_ms = 50
        self.algo_generator = None
        self.c_stats = None

        self.algos = {
            "冒泡排序": (clib.bubble_sort if clib else None, py_bubble_sort),
            "插入排序": (clib.insertion_sort if clib else None, py_insertion_sort),
            "选择排序": (clib.selection_sort if clib else None, py_selection_sort),
            "希尔排序": (clib.shell_sort if clib else None, py_shell_sort),
            "快速排序": (clib.quick_sort if clib else None, py_quick_sort_wrapper),
            "归并排序": (clib.merge_sort if clib else None, py_merge_sort_wrapper),
            "堆排序": (clib.heap_sort if clib else None, py_heap_sort),
        }

        self._setup_ui()
        self.generate_data()
        self.update_analysis_panel()  # 初始化显示面板信息

    def _setup_ui(self):
        # --- 1. 顶部信息栏 ---
        info_frame = tk.Frame(self.root, bg="#1e1e1e", height=60)
        info_frame.pack(fill=tk.X, side=tk.TOP)

        self.lbl_algo_name = tk.Label(info_frame, text="当前算法: 未选择", font=("微软雅黑", 12, "bold"), bg="#1e1e1e",
                                      fg="#00ffcc")
        self.lbl_algo_name.pack(side=tk.LEFT, padx=20, pady=10)

        stats_frame = tk.Frame(info_frame, bg="#1e1e1e")
        stats_frame.pack(side=tk.RIGHT, padx=20)
        self.lbl_stats = tk.Label(stats_frame, text="数据量: 0   比较次数: 0   移动次数: 0   耗时: 0.00ms",
                                  font=("Consolas", 10), bg="#1e1e1e", fg="white")
        self.lbl_stats.pack()

        # --- 2. 中间区域 (左侧画布 + 右侧分析面板) ---
        mid_container = tk.Frame(self.root, bg="#2b2b2b")
        mid_container.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)

        # 2.1 左侧画布
        self.canvas = tk.Canvas(mid_container, bg="#232323", highlightthickness=0)
        self.canvas.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        # 2.2 右侧算法分析面板 (新增)
        analysis_panel = tk.Frame(mid_container, bg="#333333", width=250)
        analysis_panel.pack(side=tk.RIGHT, fill=tk.Y, padx=(10, 0))
        analysis_panel.pack_propagate(False)  # 固定宽度

        tk.Label(analysis_panel, text="算法分析", font=("微软雅黑", 14, "bold"), bg="#333333", fg="#e0aa00").pack(
            pady=15)

        # 辅助函数：创建分析标签
        def create_info_row(parent, title):
            tk.Label(parent, text=title, font=("微软雅黑", 10, "bold"), bg="#333333", fg="#aaaaaa").pack(anchor="w",
                                                                                                         padx=15,
                                                                                                         pady=(10, 2))
            lbl = tk.Label(parent, text="-", font=("微软雅黑", 10), bg="#333333", fg="white", wraplength=220,
                           justify="left")
            lbl.pack(anchor="w", padx=15)
            return lbl

        self.lbl_time = create_info_row(analysis_panel, "时间复杂度")
        self.lbl_space = create_info_row(analysis_panel, "空间复杂度")
        self.lbl_stable = create_info_row(analysis_panel, "稳定性")
        self.lbl_reason = create_info_row(analysis_panel, "稳定性原因")

        # --- 3. 底部控制栏 (重构) ---
        ctrl_frame = tk.Frame(self.root, bg="#333333", height=100)
        ctrl_frame.pack(fill=tk.X, side=tk.BOTTOM)

        # 3.1 左侧：数据控制区域 (新增手动输入)
        data_ctrl_frame = tk.Frame(ctrl_frame, bg="#333333")
        data_ctrl_frame.pack(side=tk.LEFT, padx=20, pady=10)

        tk.Button(data_ctrl_frame, text="生成数据", command=self.generate_data, bg="#444", fg="white", width=12).pack(
            pady=2)
        tk.Button(data_ctrl_frame, text="手动输入", command=self.manual_input, bg="#444", fg="#b0b0b0", width=12).pack(
            pady=2)  # 新增

        # 3.2 右侧：设置区域 (数据量/算法/速度)
        settings_frame = tk.Frame(ctrl_frame, bg="#333333")
        settings_frame.pack(side=tk.RIGHT, padx=20)

        # 算法选择
        self.algo_var = tk.StringVar(value="冒泡排序")
        cb = ttk.Combobox(settings_frame, textvariable=self.algo_var, values=list(self.algos.keys()), state="readonly",
                          width=12)
        cb.pack(side=tk.LEFT, padx=5)
        cb.bind("<<ComboboxSelected>>", self.update_analysis_panel)  # 绑定事件更新右侧面板

        # 数据量滑块
        tk.Label(settings_frame, text="量:", bg="#333", fg="white").pack(side=tk.LEFT, padx=5)
        self.scale_size = tk.Scale(settings_frame, from_=10, to=200, orient=tk.HORIZONTAL, bg="#333", fg="white",
                                   length=80, showvalue=0, command=self.on_size_change)
        self.scale_size.set(50)
        self.scale_size.pack(side=tk.LEFT)

        # 速度滑块
        tk.Label(settings_frame, text="速:", bg="#333", fg="white").pack(side=tk.LEFT, padx=5)
        self.scale_speed = tk.Scale(settings_frame, from_=1, to=100, orient=tk.HORIZONTAL, bg="#333", fg="white",
                                    length=80, showvalue=0)
        self.scale_speed.set(20)
        self.scale_speed.pack(side=tk.LEFT)

        # 3.3 中间：播放控制 (按钮居中)
        play_frame = tk.Frame(ctrl_frame, bg="#333333")
        play_frame.pack(side=tk.LEFT, expand=True)  # expand=True 让它占据中间剩余空间

        btn_style = {"bg": "#007acc", "fg": "white", "font": ("Arial", 14, "bold"), "width": 4}
        tk.Button(play_frame, text="▶", command=self.start_sort, **btn_style).pack(side=tk.LEFT, padx=10)
        tk.Button(play_frame, text="⏸", command=self.pause_sort, **btn_style).pack(side=tk.LEFT, padx=10)
        tk.Button(play_frame, text="⚡", command=self.run_c_benchmark, bg="#e0aa00", fg="white",
                  font=("Arial", 14, "bold"), width=4).pack(side=tk.LEFT, padx=10)

        # 底部状态栏
        self.status_bar = tk.Label(self.root, text="就绪", bg="#2b2b2b", fg="#888", anchor="w")
        self.status_bar.pack(fill=tk.X, side=tk.BOTTOM)

    # --- 新增功能：更新分析面板 ---
    def update_analysis_panel(self, event=None):
        name = self.algo_var.get()
        info = ALGO_INFO.get(name, {})
        self.lbl_time.config(text=info.get("time", "-"))
        self.lbl_space.config(text=info.get("space", "-"))
        self.lbl_stable.config(text=info.get("stability", "-"))
        self.lbl_reason.config(text=info.get("reason", "-"))

    # --- 新增功能：手动输入 ---
    def manual_input(self):
        if self.is_sorting: return
        # 弹出输入框
        val = simpledialog.askstring("手动输入数据", "请输入整数，用逗号分隔 (例如: 10, 50, 3, 20)")
        if val:
            try:
                # 解析输入
                nums = [int(x.strip()) for x in val.split(',')]
                if len(nums) < 2:
                    messagebox.showwarning("提示", "请至少输入两个数字")
                    return
                # 更新数据
                self.data = nums
                self.original_data = nums[:]
                # 重绘
                self.draw_bars(self.data, [], -1)
                self.lbl_stats.config(text=f"数据量: {len(nums)} (手动)   比较次数: 0   移动次数: 0   耗时: 0.00ms")
            except ValueError:
                messagebox.showerror("输入错误", "格式不正确，请确保只包含数字和逗号。")

    def on_size_change(self, val):
        self.generate_data()

    def generate_data(self):
        if self.is_sorting: return
        n = self.scale_size.get()
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
        bar_w = w / max(n, 1)

        for i, val in enumerate(data):
            x0 = i * bar_w
            y0 = h - val
            x1 = x0 + bar_w - 1
            y1 = h
            color = "#4a90e2"
            if i in highlights:
                if state == 1:
                    color = "#50e3c2"
                elif state == -1:
                    color = "#e35050"
            self.canvas.create_rectangle(x0, y0, x1, y1, fill=color, outline="")

    def start_sort(self):
        if self.is_sorting: return
        self.is_sorting = True
        self.is_paused = False
        algo_name = self.algo_var.get()
        self.lbl_algo_name.config(text=f"当前算法: {algo_name}")
        self.update_analysis_panel()  # 确保面板同步
        self.data = self.original_data[:]
        _, py_func = self.algos[algo_name]
        self.algo_generator = py_func(self.data)
        self.animate()

    def pause_sort(self):
        self.is_paused = not self.is_paused

    def animate(self):
        if not self.is_sorting: return
        if self.is_paused:
            self.root.after(100, self.animate)
            return
        try:
            data_snapshot, highlights, state = next(self.algo_generator)
            self.draw_bars(data_snapshot, highlights, state)
            speed = self.scale_speed.get()
            self.root.after(speed, self.animate)
        except StopIteration:
            self.is_sorting = False
            self.draw_bars(self.data, [], 0)
            self.status_bar.config(text="排序完成！")

    def run_c_benchmark(self):
        if not clib:
            messagebox.showerror("错误", "未加载 DLL")
            return
        algo_name = self.algo_var.get()
        c_func, _ = self.algos[algo_name]
        if not c_func: return

        n = len(self.original_data)
        ArrayType = ctypes.c_int * n
        c_arr = ArrayType(*self.original_data)

        self.status_bar.config(text=f"正在调用 C 语言内核...")
        self.root.update()
        stats = c_func(c_arr, n)

        self.lbl_stats.config(
            text=f"数据量: {n}   比较次数: {stats.comparisons}   移动次数: {stats.moves}   耗时: {stats.time_ms:.4f}ms")
        self.status_bar.config(text=f"C语言内核执行完毕。耗时 {stats.time_ms:.4f} 毫秒")

        sorted_data = list(c_arr)
        self.draw_bars(sorted_data, [], 0)
        self.data = sorted_data
        self.is_sorting = False


if __name__ == "__main__":
    root = tk.Tk()
    app = VisualSortApp(root)
    root.mainloop()