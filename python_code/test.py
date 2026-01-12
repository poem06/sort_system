import tkinter as tk
from tkinter import ttk, messagebox, simpledialog, filedialog
import ctypes
import os
import random
import csv
import datetime

# 0. 配置与常量
COLORS_MAP = {
    "默认状态": "#4a90e2", "正在比较": "#f5a623",
    "交换/移动": "#d0021b", "已归位": "#7ed321"
}
COLOR_KEYS = {
    "default": COLORS_MAP["默认状态"], "compare": COLORS_MAP["正在比较"],
    "active": COLORS_MAP["交换/移动"], "sorted": COLORS_MAP["已归位"]
}

ALGO_INFO = {
    "冒泡排序": {"time": "O(n²)", "space": "O(1)", "stability": "稳定", "desc": "相邻交换，将最大值浮到末尾"},
    "插入排序": {"time": "O(n²)", "space": "O(1)", "stability": "稳定", "desc": "构建有序序列，未排序数据逐个插入"},
    "选择排序": {"time": "O(n²)", "space": "O(1)", "stability": "不稳定",
                 "desc": "每次从未排序区选最小值放到已排序区末尾"},
    "希尔排序": {"time": "O(n log n)", "space": "O(1)", "stability": "不稳定", "desc": "缩小增量排序，分组进行插入排序"},
    "快速排序": {"time": "O(n log n)", "space": "O(log n)", "stability": "不稳定",
                 "desc": "分治法，栈模拟递归分区"},
    "归并排序": {"time": "O(n log n)", "space": "O(n)", "stability": "稳定", "desc": "分治法(非递归)，自底向上合并"},
    "堆排序": {"time": "O(n log n)", "space": "O(1)", "stability": "不稳定",
               "desc": "利用堆结构特性，不断移除堆顶最大值"}
}


# 1. C语言 DLL 接口
class SortPerformance(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("algorithm", ctypes.c_char * 30),
        ("dataSize", ctypes.c_int),
        ("compareCount", ctypes.c_longlong),
        ("moveCount", ctypes.c_longlong),
        ("timeCost", ctypes.c_double)
    ]


class C_Backend:
    def __init__(self):
        try:
            dll_path = os.path.join(os.path.dirname(__file__), "Sorting_System.dll")
            self.lib = ctypes.CDLL(dll_path)
            for name in ["bubble_sort", "insertion_sort", "selection_sort",
                         "shell_sort", "quick_sort", "merge_sort", "heap_sort"]:
                if hasattr(self.lib, name):
                    func = getattr(self.lib, name)
                    func.restype = SortPerformance
                    func.argtypes = [ctypes.POINTER(ctypes.c_int), ctypes.c_int]
            if hasattr(self.lib, "generate_data_c"):
                self.lib.generate_data_c.argtypes = [ctypes.POINTER(ctypes.c_int), ctypes.c_int, ctypes.c_int]
            self.available = True
        except Exception as e:
            print(f"Warning: DLL load failed ({e})")
            self.available = False

    def generate_data(self, n, dtype):
        if not self.available: return [random.randint(0, 1000) for _ in range(n)]
        ArrayType = ctypes.c_int * n
        c_arr = ArrayType()
        self.lib.generate_data_c(c_arr, n, dtype)
        return list(c_arr)

    def run_sort(self, algo_name, data):
        if not self.available: return None
        mapping = {
            "冒泡排序": "bubble_sort", "插入排序": "insertion_sort",
            "选择排序": "selection_sort", "希尔排序": "shell_sort",
            "快速排序": "quick_sort", "归并排序": "merge_sort", "堆排序": "heap_sort"
        }
        func = getattr(self.lib, mapping.get(algo_name))
        n = len(data)
        ArrayType = ctypes.c_int * n
        c_arr = ArrayType(*data)
        stats = func(c_arr, n)
        return stats, list(c_arr)


# 2. 逻辑层：快照生成器
class Snapshot:
    def __init__(self, data, colors, text, sorted_idxs):
        self.data = list(data);
        self.colors = colors.copy();
        self.text = text;
        self.sorted_idxs = set(sorted_idxs)


class SortLogic:
    def generate_history(self, algo_name, original_data):
        data = list(original_data);
        history = [];
        n = len(data);
        sorted_indices = set()

        def add_frame(colors, msg):
            frame_colors = {};
            for i in sorted_indices: frame_colors[i] = COLOR_KEYS["sorted"]
            frame_colors.update(colors)
            history.append(Snapshot(data, frame_colors, msg, sorted_indices))

        if algo_name == "冒泡排序":
            for i in range(n - 1):
                swapped = False
                for j in range(n - 1 - i):
                    val_a, val_b = data[j], data[j + 1]
                    add_frame({j: COLOR_KEYS["compare"], j + 1: COLOR_KEYS["compare"]}, f"比较: {val_a} > {val_b} ?")
                    if val_a > val_b:
                        data[j], data[j + 1] = data[j + 1], data[j]
                        add_frame({j: COLOR_KEYS["active"], j + 1: COLOR_KEYS["active"]}, f"交换: {val_a} 与 {val_b}")
                        swapped = True
                sorted_indices.add(n - 1 - i)
                add_frame({}, f"第 {i + 1} 轮结束")
                if not swapped: break
            sorted_indices.update(range(n))
        elif algo_name == "插入排序":
            for i in range(1, n):
                key = data[i]
                j = i - 1
                add_frame({i: COLOR_KEYS["active"]}, f"取出待插入元素: {key}")
                while j >= 0:
                    add_frame({j: COLOR_KEYS["compare"], j + 1: COLOR_KEYS["active"]}, f"比较: {data[j]} > {key} ?")
                    if data[j] > key:
                        data[j + 1] = data[j]
                        add_frame({j + 1: COLOR_KEYS["active"]}, f"后移元素 {data[j + 1]}")
                        j -= 1
                    else:
                        break
                data[j + 1] = key
                add_frame({j + 1: COLOR_KEYS["active"]}, f"插入 {key} 到索引 {j + 1}")
            sorted_indices.update(range(n))

        elif algo_name == "选择排序":
            for i in range(n - 1):
                min_idx = i
                add_frame({i: COLOR_KEYS["active"]}, f"当前位置 {i}，假设最小值为 {data[i]}")
                for j in range(i + 1, n):
                    add_frame({j: COLOR_KEYS["compare"], min_idx: COLOR_KEYS["active"]},
                              f"比较: {data[j]} < {data[min_idx]} ?")
                    if data[j] < data[min_idx]:
                        min_idx = j
                        add_frame({min_idx: COLOR_KEYS["active"]}, f"发现新最小值: {data[min_idx]}")
                if min_idx != i:
                    data[i], data[min_idx] = data[min_idx], data[i]
                    add_frame({i: COLOR_KEYS["active"], min_idx: COLOR_KEYS["active"]},
                              f"交换 {data[i]} 与 {data[min_idx]}")
                sorted_indices.add(i)
            sorted_indices.update(range(n))

        elif algo_name == "希尔排序":
            gap = n // 2
            while gap > 0:
                add_frame({}, f"当前增量 Gap = {gap}")
                for i in range(gap, n):
                    temp = data[i]
                    j = i
                    add_frame({i: COLOR_KEYS["active"]}, f"分组取出: {temp}")
                    while j >= gap:
                        add_frame({j - gap: COLOR_KEYS["compare"], j: COLOR_KEYS["active"]},
                                  f"比较: {data[j - gap]} > {temp} ?")
                        if data[j - gap] > temp:
                            data[j] = data[j - gap]
                            j -= gap
                        else:
                            break
                    data[j] = temp
                    add_frame({j: COLOR_KEYS["active"]}, f"插入 {temp}")
                gap //= 2
            sorted_indices.update(range(n))

        elif algo_name == "快速排序":
            def quick(low, high):
                if low < high:
                    pivot = data[high];
                    i = low - 1
                    add_frame({high: COLOR_KEYS["active"]}, f"基准: {pivot}")
                    for j in range(low, high):
                        add_frame({j: COLOR_KEYS["compare"], high: COLOR_KEYS["active"]}, f"比较 {data[j]} vs {pivot}")
                        if data[j] < pivot:
                            i += 1;
                            data[i], data[j] = data[j], data[i]
                            add_frame({i: COLOR_KEYS["active"], j: COLOR_KEYS["active"]}, "交换")
                    data[i + 1], data[high] = data[high], data[i + 1]
                    add_frame({i + 1: COLOR_KEYS["sorted"]}, "基准归位")
                    pi = i + 1;
                    sorted_indices.add(pi)
                    quick(low, pi - 1);
                    quick(pi + 1, high)
                elif low == high:
                    sorted_indices.add(low)

            quick(0, n - 1);
            sorted_indices.update(range(n))

        elif algo_name == "归并排序":
            def merge_sort_rec(l, r):
                if l < r:
                    m = (l + r) // 2
                    merge_sort_rec(l, m)
                    merge_sort_rec(m + 1, r)

                    temp = []
                    i, j = l, m + 1
                    add_frame({l: COLOR_KEYS["active"], r: COLOR_KEYS["active"]},
                              f"准备合并区间 [{l}-{m}] 和 [{m + 1}-{r}]")
                    while i <= m and j <= r:
                        add_frame({i: COLOR_KEYS["compare"], j: COLOR_KEYS["compare"]},
                                  f"比较左({data[i]}) <= 右({data[j]}) ?")
                        if data[i] <= data[j]:
                            temp.append(data[i]);
                            i += 1
                        else:
                            temp.append(data[j]);
                            j += 1
                    while i <= m: temp.append(data[i]); i += 1
                    while j <= r: temp.append(data[j]); j += 1

                    for k, val in enumerate(temp):
                        data[l + k] = val
                        add_frame({l + k: COLOR_KEYS["active"]}, f"回填值 {val}")

            merge_sort_rec(0, n - 1)
            sorted_indices.update(range(n))

        elif algo_name == "堆排序":
            def heapify(n, i):
                largest = i
                l = 2 * i + 1
                r = 2 * i + 2
                if l < n:
                    add_frame({l: COLOR_KEYS["compare"], largest: COLOR_KEYS["active"]},
                              f"比较左子 {data[l]} > 父 {data[largest]} ?")
                    if data[l] > data[largest]: largest = l
                if r < n:
                    add_frame({r: COLOR_KEYS["compare"], largest: COLOR_KEYS["active"]},
                              f"比较右子 {data[r]} > 父 {data[largest]} ?")
                    if data[r] > data[largest]: largest = r
                if largest != i:
                    data[i], data[largest] = data[largest], data[i]
                    add_frame({i: COLOR_KEYS["active"], largest: COLOR_KEYS["active"]}, "交换父子节点")
                    heapify(n, largest)

            add_frame({}, "开始构建大顶堆")
            for i in range(n // 2 - 1, -1, -1): heapify(n, i)
            add_frame({}, "建堆完成")
            for i in range(n - 1, 0, -1):
                data[i], data[0] = data[0], data[i]
                add_frame({i: COLOR_KEYS["sorted"], 0: COLOR_KEYS["active"]}, f"最大值 {data[i]} 移到末尾")
                sorted_indices.add(i)
                heapify(i, 0)
            sorted_indices.add(0)

        else:
            sorted_indices.update(range(n));
            add_frame({}, "可视化逻辑使用默认")

        add_frame({}, "排序完成")
        return history


# 3. GUI 层
class VisualSortApp:
    def __init__(self, root):
        self.root = root
        self.root.title("排序算法可视化系统")
        self.root.geometry("1300x850")
        self.root.configure(bg="#2b2b2b")

        self.style = ttk.Style();
        self.style.theme_use('clam')
        self.style.configure("Horizontal.TProgressbar", background="#007acc", troughcolor="#333", bordercolor="#333",
                             lightcolor="#333", darkcolor="#333")
        self.style.configure("Treeview", background="#333", fieldbackground="#333", foreground="white", rowheight=25)
        self.style.configure("Treeview.Heading", background="#444", foreground="white", font=('Arial', 10, 'bold'))
        self.style.map("Treeview", background=[('selected', '#007acc')])

        self.backend = C_Backend()
        self.logic = SortLogic()

        self.data = [];
        self.history = [];
        self.current_step = 0;
        self.is_playing = False;
        self.comparison_results = [];
        self.current_data_type = "随机"
        self._setup_ui();
        self.generate_data(0)

    def _setup_ui(self):
        # 顶部
        top = tk.Frame(self.root, bg="#1e1e1e", height=50);
        top.pack(fill=tk.X)
        tk.Label(top, text="算法:", bg="#1e1e1e", fg="#aaa").pack(side=tk.LEFT, padx=10)
        self.algo_var = tk.StringVar(value="冒泡排序")
        cb = ttk.Combobox(top, textvariable=self.algo_var, values=list(ALGO_INFO.keys()), state="readonly", width=10)
        cb.pack(side=tk.LEFT, pady=10);
        cb.bind("<<ComboboxSelected>>", self.on_algo_change)

        btn_cfg = {"bg": "#444", "fg": "white", "relief": "flat", "font": ("微软雅黑", 9)}
        tk.Button(top, text="📂 导入数据", command=self.import_data, **btn_cfg).pack(side=tk.LEFT, padx=10)
        tk.Button(top, text="📊 全算法对比", command=self.run_comparison_all, bg="#e0aa00", fg="black").pack(
            side=tk.LEFT, padx=5)
        self.lbl_perf = tk.Label(top, text="C引擎: 就绪", bg="#1e1e1e", fg="#00ffcc", font=("Consolas", 10))
        self.lbl_perf.pack(side=tk.RIGHT, padx=20)

        # 中间
        mid = tk.Frame(self.root, bg="#2b2b2b");
        mid.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        self.canvas = tk.Canvas(mid, bg="#232323", highlightthickness=0);
        self.canvas.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        self.canvas.bind("<Configure>", lambda e: self.draw_frame())

        right = tk.Frame(mid, bg="#333", width=260);
        right.pack(side=tk.RIGHT, fill=tk.Y, padx=(10, 0))
        right.pack_propagate(False)
        tk.Label(right, text="算法分析", bg="#333", fg="white", font=("bold", 14)).pack(pady=(20, 10))
        self.lbl_time_c = self._create_metric(right, "时间复杂度")
        self.lbl_space_c = self._create_metric(right, "空间复杂度")
        self.lbl_stable = self._create_metric(right, "稳定性")
        self.lbl_desc = self._create_metric(right, "原理简述", wrap=True)
        tk.Frame(right, bg="#555", height=1).pack(fill=tk.X, pady=20, padx=10)
        tk.Label(right, text="颜色图例", bg="#333", fg="white", font=("bold", 12)).pack(pady=5)
        for k, v in COLORS_MAP.items():
            f = tk.Frame(right, bg="#333");
            f.pack(anchor="w", padx=20, pady=2)
            tk.Label(f, bg=v, width=2).pack(side=tk.LEFT)
            tk.Label(f, text=k, bg="#333", fg="#ccc", padx=10).pack(side=tk.LEFT)

        # 底部
        btm = tk.Frame(self.root, bg="#333", height=160);
        btm.pack(fill=tk.X, side=tk.BOTTOM)
        self.progress = ttk.Progressbar(btm, style="Horizontal.TProgressbar", orient="horizontal", mode="determinate");
        self.progress.pack(fill=tk.X, padx=0, pady=0)
        status_frame = tk.Frame(btm, bg="#252526", height=50);
        status_frame.pack(fill=tk.X, pady=(0, 5))
        self.lbl_desc_main = tk.Label(status_frame, text="Ready", bg="#252526", fg="#ffffff",
                                      font=("微软雅黑", 14, "bold"));
        self.lbl_desc_main.pack(pady=(5, 0))
        self.lbl_step_counter = tk.Label(status_frame, text="Step: 0 / 0", bg="#252526", fg="#888888",
                                         font=("Consolas", 10));
        self.lbl_step_counter.pack(pady=(0, 5))

        ctrl = tk.Frame(btm, bg="#333");
        ctrl.pack(fill=tk.BOTH, expand=True, padx=20, pady=5)
        left_box = tk.Frame(ctrl, bg="#333");
        left_box.pack(side=tk.LEFT)
        tk.Button(left_box, text="随机", command=lambda: self.generate_data(0), bg="#555", fg="white", width=5).pack(
            side=tk.LEFT, padx=2)
        tk.Button(left_box, text="正序", command=lambda: self.generate_data(1), bg="#555", fg="white", width=5).pack(
            side=tk.LEFT, padx=2)
        tk.Button(left_box, text="逆序", command=lambda: self.generate_data(2), bg="#555", fg="white", width=5).pack(
            side=tk.LEFT, padx=2)
        tk.Label(left_box, text="  数据量:", bg="#333", fg="white").pack(side=tk.LEFT)
        self.lbl_n_val = tk.Label(left_box, text="50", bg="#333", fg="cyan", width=3);
        self.lbl_n_val.pack(side=tk.LEFT)
        self.scale_n = tk.Scale(left_box, from_=10, to=300, orient=tk.HORIZONTAL, bg="#333", fg="white", showvalue=0,
                                command=self.on_scale_n);
        self.scale_n.set(50);
        self.scale_n.pack(side=tk.LEFT)

        right_box = tk.Frame(ctrl, bg="#333");
        right_box.pack(side=tk.RIGHT)
        tk.Button(right_box, text="⚡ 单项测速", command=self.run_benchmark, bg="orange", fg="black").pack(side=tk.RIGHT)
        self.scale_spd = tk.Scale(right_box, from_=200, to=10, orient=tk.HORIZONTAL, bg="#333", fg="white", showvalue=0,
                                  length=80, command=self.on_scale_spd);
        self.scale_spd.set(50);
        self.scale_spd.pack(side=tk.RIGHT, padx=(5, 20))
        self.lbl_spd_val = tk.Label(right_box, text="50ms", bg="#333", fg="cyan", width=5);
        self.lbl_spd_val.pack(side=tk.RIGHT)
        tk.Label(right_box, text="延时:", bg="#333", fg="white").pack(side=tk.RIGHT)

        center_box = tk.Frame(ctrl, bg="#333");
        center_box.place(relx=0.5, rely=0.5, anchor=tk.CENTER)
        tk.Button(center_box, text="⏮", command=self.reset).pack(side=tk.LEFT, padx=2)
        tk.Button(center_box, text="◀", command=self.prev).pack(side=tk.LEFT, padx=2)
        self.btn_play = tk.Button(center_box, text="▶", command=self.toggle, bg="#007acc", fg="white", width=5);
        self.btn_play.pack(side=tk.LEFT, padx=2)
        tk.Button(center_box, text="▶", command=self.next).pack(side=tk.LEFT, padx=2)
        self.on_algo_change()

    def _create_metric(self, parent, title, wrap=False):
        tk.Label(parent, text=title, bg="#333", fg="#888", font=("bold", 9)).pack(anchor="w", padx=15, pady=(10, 0))
        lbl = tk.Label(parent, text="-", bg="#333", fg="white", justify="left")
        if wrap: lbl.config(wraplength=230)
        lbl.pack(anchor="w", padx=15)
        return lbl

    def on_scale_n(self, val):
        self.lbl_n_val.config(text=str(val)); self.generate_data(0)

    def on_scale_spd(self, val):
        self.lbl_spd_val.config(text=f"{val}ms")

    def on_algo_change(self, e=None):
        info = ALGO_INFO.get(self.algo_var.get(), {})
        self.lbl_time_c.config(text=info.get("time"));
        self.lbl_space_c.config(text=info.get("space"))
        self.lbl_stable.config(text=info.get("stability"));
        self.lbl_desc.config(text=info.get("desc"));
        self.prepare_anim()

    def run_comparison_all(self):
        if not self.backend.available: return
        self.comparison_results = []
        top = tk.Toplevel(self.root);
        top.title(f"全算法对比 - 数据类型: {self.current_data_type}");
        top.geometry("900x550")
        frame = tk.Frame(top);
        frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)

        # 增加列：综合评分、性能评价
        cols = ("算法", "耗时(ms)", "比较次数", "移动次数", "综合评分", "性能评价")
        tree = ttk.Treeview(frame, columns=cols, show="headings")
        for col in cols: tree.heading(col, text=col, anchor="center"); tree.column(col, anchor="center", width=120)
        tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        results = []
        for algo_name in ALGO_INFO.keys():
            res = self.backend.run_sort(algo_name, self.data)
            if res: results.append((algo_name, res[0]))

        if results:
            min_time = min(r[1].timeCost for r in results)
            if min_time <= 0: min_time = 0.000001  # 避免除0

            for algo_name, stats in results:
                # 计算评分
                score = 100 * (min_time / stats.timeCost) if stats.timeCost > 0 else 0
                eval_str = "优秀" if score >= 90 else "良好" if score >= 60 else "一般" if score >= 20 else "较差"

                # 记录详细结果用于导出
                self.comparison_results.append({
                    "algo": algo_name, "time": stats.timeCost, "comp": stats.compareCount,
                    "move": stats.moveCount, "score": score, "eval": eval_str
                })

                tree.insert("", tk.END, values=(
                    algo_name, f"{stats.timeCost:.6f}", stats.compareCount, stats.moveCount,
                    f"{score:.1f}", eval_str
                ))

        tk.Button(top, text="导出此表格", command=self.export_report, height=2, bg="#e0aa00").pack(pady=10, fill=tk.X,
                                                                                                   padx=20)

    def export_report(self):
        if not self.comparison_results: messagebox.showwarning("提示", "请先运行"); return
        path = filedialog.asksaveasfilename(defaultextension=".csv", filetypes=[("CSV", "*.csv")])
        if not path: return
        try:
            with open(path, 'w', newline='', encoding='utf-8-sig') as f:
                writer = csv.writer(f)
                writer.writerow(
                    ["时间", "数据类型", "算法", "数据量", "耗时(ms)", "比较次数", "移动次数", "综合评分", "性能评价"])
                now = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                for item in self.comparison_results:
                    writer.writerow([
                        now, self.current_data_type, item["algo"], len(self.data),
                        f"{item['time']:.6f}", item["comp"], item["move"], f"{item['score']:.1f}", item["eval"]
                    ])
            messagebox.showinfo("成功", "已导出")
        except Exception as e:
            messagebox.showerror("错误", str(e))

    def run_benchmark(self):
        if not self.backend.available: return
        res = self.backend.run_sort(self.algo_var.get(), self.data)
        if not res: return
        stats, sorted_data = res
        self.lbl_perf.config(text=f"C测速: {stats.timeCost:.6f}ms | 比较 {stats.compareCount} | 移动 {stats.moveCount}")
        self.data = sorted_data;
        self.prepare_anim()
        self.current_step = len(self.history) - 1;
        self.draw_frame()

    def import_data(self):
        path = filedialog.askopenfilename()
        if not path: return
        try:
            with open(path, 'r') as f:
                nums = [int(x) for x in f.read().replace(',', ' ').split() if x.strip().isdigit()]
                if nums: self.data = nums; self.current_data_type = "导入"; self.lbl_n_val.config(
                    text=str(len(nums))); self.prepare_anim()
        except Exception as e:
            messagebox.showerror("错误", str(e))

    def generate_data(self, type_code):
        self.current_data_type = ["随机", "正序", "逆序"][type_code]
        if self.backend.available:
            self.data = self.backend.generate_data(self.scale_n.get(), type_code)
        else:
            self.data = [random.randint(0, 800) for _ in range(self.scale_n.get())]
        self.prepare_anim()

    def prepare_anim(self):
        self.is_playing = False;
        self.btn_play.config(text="▶")
        self.history = self.logic.generate_history(self.algo_var.get(), self.data)
        self.current_step = 0;
        self.progress['maximum'] = len(self.history) - 1;
        self.progress['value'] = 0;
        self.draw_frame()

    def draw_frame(self):
        self.canvas.delete("all")
        if not self.history: return
        snap = self.history[self.current_step]
        self.lbl_desc_main.config(text=snap.text if snap.text else "Ready")
        self.lbl_step_counter.config(text=f"Step: {self.current_step} / {len(self.history) - 1}")
        self.progress['value'] = self.current_step
        w = self.canvas.winfo_width();
        h = self.canvas.winfo_height()
        n = len(snap.data);
        bar_w = w / max(n, 1);
        max_val = max(snap.data) if snap.data else 1
        for i, val in enumerate(snap.data):
            x0 = i * bar_w;
            y0 = h - (val / max_val * (h - 20))
            self.canvas.create_rectangle(x0, y0, x0 + bar_w - 1, h, fill=snap.colors.get(i, COLOR_KEYS["default"]),
                                         outline="")

    def toggle(self):
        self.is_playing = not self.is_playing; self.btn_play.config(
            text="⏸" if self.is_playing else "▶"); self.run_anim()

    def run_anim(self):
        if self.is_playing and self.current_step < len(self.history) - 1:
            self.current_step += 1;
            self.draw_frame();
            self.root.after(self.scale_spd.get(), self.run_anim)
        else:
            self.is_playing = False; self.btn_play.config(text="▶")

    def next(self):
        if self.current_step < len(self.history) - 1: self.current_step += 1; self.draw_frame()

    def prev(self):
        if self.current_step > 0: self.current_step -= 1; self.draw_frame()

    def reset(self):
        self.current_step = 0; self.draw_frame()


if __name__ == "__main__":
    root = tk.Tk();
    VisualSortApp(root);
    root.mainloop()
