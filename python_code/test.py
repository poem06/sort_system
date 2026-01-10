import tkinter as tk
from tkinter import ttk, messagebox, simpledialog
import ctypes
import os
import random
import copy

# ==========================================
# 0. 配置与常量
# ==========================================
COLOR_DEFAULT = "#4a90e2"  # 蓝色
COLOR_COMPARE = "#f5a623"  # 黄色
COLOR_ACTIVE = "#d0021b"  # 红色
COLOR_SORTED = "#7ed321"  # 绿色

ALGO_INFO = {
    "冒泡排序": {"time": "O(n²)", "space": "O(1)", "stability": "稳定", "reason": "相邻交换，相等不换。"},
    "插入排序": {"time": "O(n²)", "space": "O(1)", "stability": "稳定", "reason": "从后向前扫描插入，相等插在后。"},
    "选择排序": {"time": "O(n²)", "space": "O(1)", "stability": "不稳定", "reason": "远距离交换可能跨越相等元素。"},
    "希尔排序": {"time": "O(n log n)~O(n²)", "space": "O(1)", "stability": "不稳定",
                 "reason": "分组跳跃式移动破坏相对顺序。"},
    "快速排序": {"time": "O(n log n)", "space": "O(log n)", "stability": "不稳定",
                 "reason": "基准值交换涉及远距离跳跃。"},
    "归并排序": {"time": "O(n log n)", "space": "O(n)", "stability": "稳定", "reason": "合并时左侧优先。"},
    "堆排序": {"time": "O(n log n)", "space": "O(1)", "stability": "不稳定", "reason": "堆调整破坏相对顺序。"}
}


# ==========================================
# 1. C语言 DLL 接口
# ==========================================
class SortStats(ctypes.Structure):
    _fields_ = [("comparisons", ctypes.c_longlong), ("moves", ctypes.c_longlong), ("time_ms", ctypes.c_double)]


try:
    dll_path = os.path.join(os.path.dirname(__file__), "Sorting_ASystem.dll")
    clib = ctypes.CDLL(dll_path)
    if clib:
        clib.bubble_sort.restype = SortStats
        clib.insertion_sort.restype = SortStats
        clib.selection_sort.restype = SortStats
        clib.shell_sort.restype = SortStats
        clib.quick_sort.restype = SortStats
        clib.merge_sort.restype = SortStats
        clib.heap_sort.restype = SortStats
        clib.generate_data.argtypes = [ctypes.POINTER(ctypes.c_int), ctypes.c_int, ctypes.c_int]
except OSError:
    clib = None
    print("警告: 未找到 DLL，极速模式不可用。")


# ==========================================
# 2. 核心逻辑：快照生成器
# ==========================================
class SortSnapshot:
    def __init__(self, data, colors, text, sorted_indices):
        self.data = list(data)
        self.colors = colors.copy()
        self.text = text
        self.sorted_indices = set(sorted_indices)


class SortLogic:
    def generate_history(self, algo_name, original_data):
        data = list(original_data)
        history = []
        n = len(data)
        sorted_indices = set()

        def add_frame(colors, msg):
            frame_colors = {}
            for i in sorted_indices: frame_colors[i] = COLOR_SORTED
            frame_colors.update(colors)
            history.append(SortSnapshot(data, frame_colors, msg, sorted_indices))

        if algo_name == "冒泡排序":
            for i in range(n - 1):
                swapped = False
                for j in range(n - i - 1):
                    add_frame({j: COLOR_COMPARE, j + 1: COLOR_COMPARE}, f"比较: {data[j]} vs {data[j + 1]}")
                    if data[j] > data[j + 1]:
                        data[j], data[j + 1] = data[j + 1], data[j]
                        add_frame({j: COLOR_ACTIVE, j + 1: COLOR_ACTIVE}, f"交换")
                        swapped = True
                sorted_indices.add(n - 1 - i)
                add_frame({}, f"第 {i + 1} 轮结束")
                if not swapped: break
            sorted_indices.update(range(n))

        elif algo_name == "插入排序":
            for i in range(1, n):
                key = data[i]
                j = i - 1
                add_frame({i: COLOR_ACTIVE}, f"取出 {key}")
                while j >= 0 and data[j] > key:
                    add_frame({j: COLOR_COMPARE, j + 1: COLOR_ACTIVE}, f"后移")
                    data[j + 1] = data[j]
                    add_frame({j: COLOR_DEFAULT, j + 1: COLOR_ACTIVE}, f"移动")
                    j -= 1
                data[j + 1] = key
                add_frame({j + 1: COLOR_ACTIVE}, f"插入 {key}")
            sorted_indices.update(range(n))

        elif algo_name == "选择排序":
            for i in range(n - 1):
                min_idx = i
                add_frame({i: COLOR_ACTIVE}, f"假设最小: {data[i]}")
                for j in range(i + 1, n):
                    add_frame({j: COLOR_COMPARE, min_idx: COLOR_ACTIVE}, f"寻找最小")
                    if data[j] < data[min_idx]:
                        min_idx = j
                        add_frame({min_idx: COLOR_ACTIVE}, f"新最小: {data[min_idx]}")
                if min_idx != i:
                    data[i], data[min_idx] = data[min_idx], data[i]
                    add_frame({i: COLOR_ACTIVE, min_idx: COLOR_ACTIVE}, f"交换")
                sorted_indices.add(i)
            sorted_indices.update(range(n))

        elif algo_name == "快速排序":
            def quick_sort(low, high):
                if low < high:
                    pivot = data[high]
                    i = low - 1
                    add_frame({high: COLOR_ACTIVE}, f"基准: {pivot}")
                    for j in range(low, high):
                        add_frame({j: COLOR_COMPARE, high: COLOR_ACTIVE}, f"比较")
                        if data[j] < pivot:
                            i += 1
                            data[i], data[j] = data[j], data[i]
                            add_frame({i: COLOR_ACTIVE, j: COLOR_ACTIVE}, f"交换到左侧")
                    data[i + 1], data[high] = data[high], data[i + 1]
                    add_frame({i + 1: COLOR_SORTED}, f"归位")
                    pi = i + 1
                    sorted_indices.add(pi)
                    quick_sort(low, pi - 1)
                    quick_sort(pi + 1, high)
                elif low == high:
                    sorted_indices.add(low)

            quick_sort(0, n - 1)
            sorted_indices.update(range(n))

        elif algo_name == "希尔排序":
            gap = n // 2
            while gap > 0:
                add_frame({}, f"增量 Gap: {gap}")
                for i in range(gap, n):
                    temp = data[i]
                    j = i
                    add_frame({i: COLOR_ACTIVE}, f"取出 {temp}")
                    while j >= gap and data[j - gap] > temp:
                        add_frame({j - gap: COLOR_COMPARE, j: COLOR_ACTIVE}, f"后移")
                        data[j] = data[j - gap]
                        j -= gap
                    data[j] = temp
                    add_frame({j: COLOR_ACTIVE}, f"插入")
                gap //= 2
            sorted_indices.update(range(n))

        elif algo_name == "堆排序":
            def heapify(n, i):
                largest = i
                l = 2 * i + 1;
                r = 2 * i + 2
                if l < n:
                    add_frame({l: COLOR_COMPARE, largest: COLOR_ACTIVE}, f"比较左子")
                    if data[l] > data[largest]: largest = l
                if r < n:
                    add_frame({r: COLOR_COMPARE, largest: COLOR_ACTIVE}, f"比较右子")
                    if data[r] > data[largest]: largest = r
                if largest != i:
                    data[i], data[largest] = data[largest], data[i]
                    add_frame({i: COLOR_ACTIVE, largest: COLOR_ACTIVE}, "交换堆顶")
                    heapify(n, largest)

            for i in range(n // 2 - 1, -1, -1): heapify(n, i)
            add_frame({}, "建堆完成")
            for i in range(n - 1, 0, -1):
                data[i], data[0] = data[0], data[i]
                add_frame({0: COLOR_ACTIVE, i: COLOR_SORTED}, f"移出最大值")
                sorted_indices.add(i)
                heapify(i, 0)
            sorted_indices.add(0)

        elif algo_name == "归并排序":
            def merge(l, m, r):
                n1 = m - l + 1
                n2 = r - m
                L = data[l:m + 1]
                R = data[m + 1:r + 1]
                i = 0;
                j = 0;
                k = l
                while i < n1 and j < n2:
                    add_frame({k: COLOR_ACTIVE}, f"合并")
                    if L[i] <= R[j]:
                        data[k] = L[i]; i += 1
                    else:
                        data[k] = R[j]; j += 1
                    k += 1
                while i < n1: data[k] = L[i]; i += 1; k += 1; add_frame({k - 1: COLOR_ACTIVE}, "回填")
                while j < n2: data[k] = R[j]; j += 1; k += 1; add_frame({k - 1: COLOR_ACTIVE}, "回填")

            def merge_sort(l, r):
                if l < r:
                    m = (l + r) // 2
                    merge_sort(l, m)
                    merge_sort(m + 1, r)
                    merge(l, m, r)

            merge_sort(0, n - 1)
            sorted_indices.update(range(n))

        add_frame({}, "排序完成！")
        return history


# ==========================================
# 3. GUI 主程序
# ==========================================
class VisualSortApp:
    def __init__(self, root):
        self.root = root
        self.root.title("排序算法可视化系统")
        self.root.geometry("1200x800")
        self.root.configure(bg="#2b2b2b")

        # 绑定全屏快捷键
        self.root.bind("<F11>", self.toggle_fullscreen)
        self.root.bind("<Escape>", self.exit_fullscreen)

        self.logic = SortLogic()
        self.data = []
        self.history = []
        self.current_step = 0
        self.is_playing = False

        self._setup_ui()
        self.generate_data()
        self.on_algo_change()

    # --- 全屏切换逻辑 ---
    def toggle_fullscreen(self, event=None):
        self.root.attributes("-fullscreen", not self.root.attributes("-fullscreen"))
        return "break"

    def exit_fullscreen(self, event=None):
        self.root.attributes("-fullscreen", False)
        return "break"

    def _setup_ui(self):
        # A. 顶部
        top_bar = tk.Frame(self.root, bg="#1e1e1e", height=50)
        top_bar.pack(fill=tk.X, side=tk.TOP)

        tk.Label(top_bar, text="当前算法:", bg="#1e1e1e", fg="#aaa", font=("微软雅黑", 10)).pack(side=tk.LEFT, padx=15)
        self.algo_var = tk.StringVar(value="冒泡排序")
        algo_cb = ttk.Combobox(top_bar, textvariable=self.algo_var, values=list(ALGO_INFO.keys()), state="readonly",
                               width=12)
        algo_cb.pack(side=tk.LEFT, pady=10)
        algo_cb.bind("<<ComboboxSelected>>", self.on_algo_change)

        self.lbl_c_stats = tk.Label(top_bar, text="C语言测速: 待机", bg="#1e1e1e", fg="#00ffcc", font=("Consolas", 10))
        self.lbl_c_stats.pack(side=tk.RIGHT, padx=20)

        # B. 中间
        mid_frame = tk.Frame(self.root, bg="#2b2b2b")
        mid_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)

        self.canvas = tk.Canvas(mid_frame, bg="#232323", highlightthickness=0)
        self.canvas.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        # 【关键修复】绑定窗口大小改变事件 -> 自动重绘
        self.canvas.bind("<Configure>", self.on_canvas_resize)

        right_panel = tk.Frame(mid_frame, bg="#333", width=260)
        right_panel.pack(side=tk.RIGHT, fill=tk.Y, padx=(10, 0))
        right_panel.pack_propagate(False)

        tk.Label(right_panel, text="图例", bg="#333", fg="white", font=("微软雅黑", 12, "bold")).pack(pady=(15, 10))
        self._add_legend(right_panel, COLOR_DEFAULT, "默认")
        self._add_legend(right_panel, COLOR_COMPARE, "比较")
        self._add_legend(right_panel, COLOR_ACTIVE, "交换/覆盖")
        self._add_legend(right_panel, COLOR_SORTED, "归位")

        tk.Frame(right_panel, bg="#555", height=1).pack(fill=tk.X, pady=20, padx=10)

        tk.Label(right_panel, text="算法分析", bg="#333", fg="white", font=("微软雅黑", 12, "bold")).pack(pady=(0, 10))
        self.lbl_time = self._add_info(right_panel, "时间复杂度")
        self.lbl_space = self._add_info(right_panel, "空间复杂度")
        self.lbl_stable = self._add_info(right_panel, "稳定性")
        self.lbl_reason = self._add_info(right_panel, "原因", wrap=True)

        # C. 底部
        btm_frame = tk.Frame(self.root, bg="#333", height=140)
        btm_frame.pack(fill=tk.X, side=tk.BOTTOM)

        self.lbl_explain = tk.Label(btm_frame, text="准备就绪", bg="#222", fg="#ffdfba", font=("微软雅黑", 11),
                                    height=2)
        self.lbl_explain.pack(fill=tk.X, side=tk.TOP, pady=2)

        ctrl_line = tk.Frame(btm_frame, bg="#333")
        ctrl_line.pack(fill=tk.BOTH, expand=True, padx=20, pady=5)

        # 左侧
        left_box = tk.Frame(ctrl_line, bg="#333")
        left_box.pack(side=tk.LEFT)
        tk.Button(left_box, text="生成随机数据", command=self.generate_data, bg="#555", fg="white").pack(side=tk.LEFT,
                                                                                                         padx=5)
        tk.Button(left_box, text="手动输入...", command=self.manual_input, bg="#555", fg="white").pack(side=tk.LEFT,
                                                                                                       padx=5)

        # 中间
        center_box = tk.Frame(ctrl_line, bg="#333")
        center_box.pack(side=tk.LEFT, expand=True)
        tk.Button(center_box, text="⏮", command=self.reset_step).pack(side=tk.LEFT, padx=5)
        tk.Button(center_box, text="◀ 上一步", command=self.prev_step, bg="#007acc", fg="white").pack(side=tk.LEFT,
                                                                                                      padx=5)
        self.btn_play = tk.Button(center_box, text="▶ 播放", command=self.toggle_play, bg="#007acc", fg="white",
                                  width=8)
        self.btn_play.pack(side=tk.LEFT, padx=5)
        tk.Button(center_box, text="下一步 ▶", command=self.next_step, bg="#007acc", fg="white").pack(side=tk.LEFT,
                                                                                                      padx=5)

        # 右侧
        right_box = tk.Frame(ctrl_line, bg="#333")
        right_box.pack(side=tk.RIGHT)
        tk.Label(right_box, text="数据量:", bg="#333", fg="white").pack(side=tk.LEFT)
        self.scale_size = tk.Scale(right_box, from_=10, to=300, orient=tk.HORIZONTAL, bg="#333", fg="white",
                                   showvalue=0, length=80, command=self.on_size_change)
        self.scale_size.set(50)
        self.scale_size.pack(side=tk.LEFT, padx=(0, 15))
        tk.Label(right_box, text="速度:", bg="#333", fg="white").pack(side=tk.LEFT)
        self.scale_speed = tk.Scale(right_box, from_=200, to=10, orient=tk.HORIZONTAL, bg="#333", fg="white",
                                    showvalue=0, length=80)
        self.scale_speed.set(50)
        self.scale_speed.pack(side=tk.LEFT, padx=5)
        tk.Button(right_box, text="⚡ 极速测速", command=self.run_c_benchmark, bg="#e0aa00", fg="white").pack(
            side=tk.LEFT, padx=10)

        # 初始化最后做
        self.on_algo_change()

    # --- 辅助方法 ---
    def _add_legend(self, parent, color, text):
        f = tk.Frame(parent, bg="#333")
        f.pack(anchor="w", padx=20, pady=2)
        tk.Label(f, bg=color, width=2).pack(side=tk.LEFT)
        tk.Label(f, text=text, bg="#333", fg="#ccc", padx=10).pack(side=tk.LEFT)

    def _add_info(self, parent, title, wrap=False):
        tk.Label(parent, text=title, bg="#333", fg="#888", font=("微软雅黑", 9, "bold")).pack(anchor="w", padx=20,
                                                                                              pady=(5, 0))
        lbl = tk.Label(parent, text="-", bg="#333", fg="white", font=("微软雅黑", 10), justify="left")
        if wrap: lbl.config(wraplength=220)
        lbl.pack(anchor="w", padx=20)
        return lbl

    # --- 事件处理 ---
    def on_canvas_resize(self, event):
        """窗口大小改变时自动重绘"""
        self.draw_frame()

    def on_algo_change(self, event=None):
        name = self.algo_var.get()
        info = ALGO_INFO.get(name, {})
        self.lbl_time.config(text=info.get("time", "-"))
        self.lbl_space.config(text=info.get("space", "-"))
        self.lbl_stable.config(text=info.get("stability", "-"))
        self.lbl_reason.config(text=info.get("reason", "-"))
        self.prepare_animation()

    def on_size_change(self, val):
        self.generate_data()

    def generate_data(self):
        try:
            n = self.scale_size.get()
        except:
            n = 50
        # 初始高度可以大一点，反正 draw_frame 会归一化
        h = 800
        self.data = [random.randint(10, h) for _ in range(n)]
        self.prepare_animation()

    def manual_input(self):
        val = simpledialog.askstring("手动输入", "例如: 50, 100, 20, 5")
        if val:
            try:
                nums = [int(x.strip()) for x in val.split(',')]
                if len(nums) < 2: return
                self.data = nums
                self.prepare_animation()
            except:
                messagebox.showerror("错误", "输入格式有误")

    def prepare_animation(self):
        self.is_playing = False
        self.btn_play.config(text="▶ 播放")
        self.current_step = 0
        algo = self.algo_var.get()
        self.history = self.logic.generate_history(algo, self.data)
        init_snap = SortSnapshot(self.data, {}, "初始状态", set())
        self.history.insert(0, init_snap)
        self.draw_frame()

    def draw_frame(self):
        self.canvas.delete("all")
        if not self.history: return
        snap = self.history[self.current_step]
        self.lbl_explain.config(text=f"[步骤 {self.current_step}/{len(self.history) - 1}] {snap.text}")

        # 实时获取当前画布宽高
        w = self.canvas.winfo_width()
        h = self.canvas.winfo_height()
        n = len(snap.data)

        bar_w = w / max(n, 1)
        max_val = max(snap.data) if snap.data and max(snap.data) > 0 else 1

        # 根据当前高度动态缩放
        draw_h = h - 40

        for i, val in enumerate(snap.data):
            x0 = i * bar_w
            # 这里的 val / max_val * draw_h 保证了无论窗口多大，柱子都按比例充满
            y0 = h - 20 - (val / max_val * draw_h)
            x1 = x0 + bar_w - 1 if bar_w > 4 else x0 + bar_w
            y1 = h - 20
            color = snap.colors.get(i, COLOR_DEFAULT)
            self.canvas.create_rectangle(x0, y0, x1, y1, fill=color, outline="")
            if n < 40:
                self.canvas.create_text((x0 + x1) / 2, y0 - 10, text=str(val), fill="#eee", font=("Arial", 8))

    # --- 控制 ---
    def toggle_play(self):
        if self.is_playing:
            self.is_playing = False
            self.btn_play.config(text="▶ 播放")
        else:
            if self.current_step >= len(self.history) - 1:
                self.current_step = 0
            self.is_playing = True
            self.btn_play.config(text="⏸ 暂停")
            self.auto_play()

    def auto_play(self):
        if self.is_playing and self.current_step < len(self.history) - 1:
            self.current_step += 1
            self.draw_frame()
            ms = self.scale_speed.get()
            self.root.after(ms, self.auto_play)
        elif self.is_playing:
            self.is_playing = False
            self.btn_play.config(text="▶ 播放")

    def next_step(self):
        if self.current_step < len(self.history) - 1:
            self.current_step += 1
            self.draw_frame()

    def prev_step(self):
        if self.current_step > 0:
            self.current_step -= 1
            self.draw_frame()

    def reset_step(self):
        self.current_step = 0
        self.is_playing = False
        self.btn_play.config(text="▶ 播放")
        self.draw_frame()

    def run_c_benchmark(self):
        if not clib:
            messagebox.showerror("错误", "DLL未连接")
            return
        algo_name = self.algo_var.get()
        func_map = {
            "冒泡排序": clib.bubble_sort, "插入排序": clib.insertion_sort,
            "选择排序": clib.selection_sort, "希尔排序": clib.shell_sort,
            "快速排序": clib.quick_sort, "归并排序": clib.merge_sort, "堆排序": clib.heap_sort
        }
        c_func = func_map.get(algo_name)
        if not c_func: return
        n = len(self.data)
        ArrayType = ctypes.c_int * n
        c_arr = ArrayType(*self.data)
        stats = c_func(c_arr, n)
        self.lbl_c_stats.config(
            text=f"C测速: 耗时 {stats.time_ms:.4f}ms | 比较 {stats.comparisons} | 移动 {stats.moves}")
        self.data = list(c_arr)
        self.prepare_animation()
        self.current_step = len(self.history) - 1
        self.draw_frame()
        self.lbl_explain.config(text=f"C语言极速排序完成！(耗时 {stats.time_ms:.4f}ms)")


if __name__ == "__main__":
    root = tk.Tk()
    app = VisualSortApp(root)
    root.mainloop()