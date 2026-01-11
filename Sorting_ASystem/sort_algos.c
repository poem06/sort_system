#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <windows.h> // 用于Sleep, 清屏, 高精度计时

// ==========================================
// 1. 数据结构与全局变量 
// ==========================================
typedef struct SortPerformance {
    char algorithm[20];    // 算法名称
    int dataSize;          // 数据规模
    long compareCount;     // 比较次数
    long moveCount;        // 移动次数
    double timeCost;       // 执行耗时（秒）
} SortPerformance;

// 全局统计变量
long g_compareCount = 0;
long g_moveCount = 0;
int g_visualize = 0;       // 可视化开关
int g_delay = 50;          // 延时(ms)

// 高精度计时频率
LARGE_INTEGER g_cpuFreq;

// ==========================================
// 2. 辅助工具函数
// ==========================================

// 初始化计时器
void initTimer() {
    QueryPerformanceFrequency(&g_cpuFreq);
}

// 获取当前时间戳
long long getTick() {
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);
    return t.QuadPart;
}

// 计算耗时(秒)
double calcDuration(long long start, long long end) {
    return (double)(end - start) / g_cpuFreq.QuadPart;
}

// 可视化展示 
void visualize(int arr[], int n) {
    if (!g_visualize) return;

    system("cls");
    printf("--- 排序可视化中 ---\n");
    for (int i = 0; i < n; i++) {
        printf("%3d | ", arr[i]);
        if (n <= 30) {
            for (int k = 0; k < arr[i]; k++) printf("*");
        }
        printf("\n");
    }
    Sleep(g_delay);
}

// 交换元素
void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
    g_moveCount += 3;
}

// 数组复制
void copyArray(int src[], int dest[], int n) {
    for (int i = 0; i < n; i++) dest[i] = src[i];
}

// ==========================================
// 3. 排序算法实现 
// ==========================================

// --- 基础算法 ---

// 冒泡排序 
void BubbleSort(int arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1 - i; j++) {
            g_compareCount++;
            if (arr[j] > arr[j + 1]) {
                swap(&arr[j], &arr[j + 1]);
                visualize(arr, n);
            }
        }
    }
}

// 直接插入排序 
void InsertSort(int arr[], int n) {
    int i, j, temp;
    for (i = 1; i < n; i++) {
        temp = arr[i];
        g_moveCount++;
        j = i - 1;
        g_compareCount++;
        while (j >= 0 && arr[j] > temp) {
            g_compareCount++;
            arr[j + 1] = arr[j];
            g_moveCount++;
            j--;
            visualize(arr, n);
        }
        arr[j + 1] = temp;
        g_moveCount++;
        visualize(arr, n);
    }
}

// 直接选择排序 
void SelectSort(int arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        int minIdx = i;
        for (int j = i + 1; j < n; j++) {
            g_compareCount++;
            if (arr[j] < arr[minIdx]) {
                minIdx = j;
            }
        }
        if (minIdx != i) {
            swap(&arr[i], &arr[minIdx]);
            visualize(arr, n);
        }
    }
}

// --- 进阶算法 (新增) ---

// 希尔排序 
void ShellSort(int arr[], int n) {
    int gap, i, j, temp;
    // 增量序列：n/2, n/4, ... 1
    for (gap = n / 2; gap > 0; gap /= 2) {
        for (i = gap; i < n; i++) {
            temp = arr[i];
            g_moveCount++; // 暂存
            j = i;

            // 组内直接插入排序
            g_compareCount++;
            while (j >= gap && arr[j - gap] > temp) {
                g_compareCount++;
                arr[j] = arr[j - gap];
                g_moveCount++;
                j -= gap;
                visualize(arr, n);
            }
            arr[j] = temp;
            g_moveCount++;
        }
        visualize(arr, n); // 每轮增量完成后展示
    }
}

// 堆排序辅助函数：堆调整
void Heapify(int arr[], int n, int i) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    g_compareCount++;
    if (left < n && arr[left] > arr[largest])
        largest = left;

    g_compareCount++;
    if (right < n && arr[right] > arr[largest])
        largest = right;

    if (largest != i) {
        swap(&arr[i], &arr[largest]);
        visualize(arr, n);
        Heapify(arr, n, largest);
    }
}

// 堆排序 
void HeapSort(int arr[], int n) {
    // 建堆
    for (int i = n / 2 - 1; i >= 0; i--)
        Heapify(arr, n, i);

    // 排序
    for (int i = n - 1; i > 0; i--) {
        swap(&arr[0], &arr[i]); // 将堆顶元素移到末尾
        visualize(arr, n);
        Heapify(arr, i, 0); // 调整剩余堆
    }
}

// 归并排序辅助函数：合并 
void Merge(int arr[], int l, int m, int r, int temp[]) {
    int i = l;
    int j = m + 1;
    int k = l;

    while (i <= m && j <= r) {
        g_compareCount++;
        if (arr[i] <= arr[j]) {
            temp[k++] = arr[i++];
            g_moveCount++; // 移动到临时数组
        }
        else {
            temp[k++] = arr[j++];
            g_moveCount++;
        }
    }
    while (i <= m) {
        temp[k++] = arr[i++];
        g_moveCount++;
    }
    while (j <= r) {
        temp[k++] = arr[j++];
        g_moveCount++;
    }
    // 拷贝回原数组
    for (i = l; i <= r; i++) {
        arr[i] = temp[i];
        g_moveCount++;
    }
    // 归并排序的可视化通常在合并后展示较为清晰
    // visualize(arr, ...); // 由于是局部更新，这里省略频繁调用以免闪屏太快
}

// 归并排序递归部分
void MergeSortRecursive(int arr[], int l, int r, int temp[], int n_total) {
    if (l < r) {
        int m = l + (r - l) / 2;
        MergeSortRecursive(arr, l, m, temp, n_total);
        MergeSortRecursive(arr, m + 1, r, temp, n_total);
        Merge(arr, l, m, r, temp);
        visualize(arr, n_total); // 每次合并完成后展示
    }
}

// 归并排序入口
void MergeSort(int arr[], int n) {
    int* temp = (int*)malloc(n * sizeof(int));
    if (temp) {
        MergeSortRecursive(arr, 0, n - 1, temp, n);
        free(temp);
    }
}

// 快速排序 (递归版) 
int Partition(int arr[], int low, int high, int n_visual) {
    int pivot = arr[low];
    g_moveCount++;
    while (low < high) {
        while (low < high && arr[high] >= pivot) {
            g_compareCount++; high--;
        }
        g_compareCount++;
        arr[low] = arr[high]; g_moveCount++;
        while (low < high && arr[low] <= pivot) {
            g_compareCount++; low++;
        }
        g_compareCount++;
        arr[high] = arr[low]; g_moveCount++;
        visualize(arr, n_visual);
    }
    arr[low] = pivot; g_moveCount++;
    return low;
}
void QuickSortRecursive(int arr[], int low, int high, int n_visual) {
    if (low < high) {
        int pivot = Partition(arr, low, high, n_visual);
        QuickSortRecursive(arr, low, pivot - 1, n_visual);
        QuickSortRecursive(arr, pivot + 1, high, n_visual);
    }
}
void QuickSort(int arr[], int n) {
    QuickSortRecursive(arr, 0, n - 1, n);
}

// ==========================================
// 4. 数据管理模块 
// ==========================================

// 生成测试数据
void generateData(int arr[], int n, int type) {
    srand((unsigned)time(NULL));
    for (int i = 0; i < n; i++) arr[i] = rand() % 100 + 1; // 范围扩大到100
    if (type == 1) { // 有序
        for (int i = 0;i < n - 1;i++) for (int j = 0;j < n - 1 - i;j++) if (arr[j] > arr[j + 1]) { int t = arr[j];arr[j] = arr[j + 1];arr[j + 1] = t; }
    }
    else if (type == 2) { // 逆序
        for (int i = 0;i < n - 1;i++) for (int j = 0;j < n - 1 - i;j++) if (arr[j] < arr[j + 1]) { int t = arr[j];arr[j] = arr[j + 1];arr[j + 1] = t; }
    }
}

// 批量导入模块：从文件读取 
// 文件格式假设：第一行是数字个数N，第二行是N个空格分隔的整数
int* importFromFile(int* n) {
    char filename[100];
    printf("请输入文件名 (例如 data.txt): ");
    scanf_s("%s", filename, (unsigned)sizeof(filename));

    FILE* fp;
    if (fopen_s(&fp, filename, "r") != 0) {
        printf("错误：无法打开文件 %s\n", filename);
        return NULL;
    }

    if (fscanf_s(fp, "%d", n) != 1) {
        printf("错误：文件格式不正确 (首行应为数字个数)\n");
        fclose(fp);
        return NULL;
    }

    int* arr = (int*)malloc((*n) * sizeof(int));
    for (int i = 0; i < *n; i++) {
        if (fscanf_s(fp, "%d", &arr[i]) != 1) {
            printf("警告：数据读取不完整\n");
            break;
        }
    }
    fclose(fp);
    printf("成功导入 %d 个数据。\n", *n);
    return arr;
}

// 导出结果模块 
void saveResultsToFile(SortPerformance results[], int count) {
    FILE* fp;
    char filename[] = "sort_report.csv"; // 导出为 CSV，Excel可直接打开

    // 使用 "a" (append) 模式追加写入，或者 "w" 覆盖
    if (fopen_s(&fp, filename, "a") != 0) {
        printf("错误：无法保存文件\n");
        return;
    }

    // 如果文件是空的，先写入表头
    fseek(fp, 0, SEEK_END);
    if (ftell(fp) == 0) {
        fprintf(fp, "Algorithm,DataSize,Comparisons,Moves,Time(s)\n");
    }

    for (int i = 0; i < count; i++) {
        fprintf(fp, "%s,%d,%ld,%ld,%.8f\n",
            results[i].algorithm, results[i].dataSize,
            results[i].compareCount, results[i].moveCount, results[i].timeCost);
    }

    fclose(fp);
    printf("结果已导出至 %s\n", filename);
}

// ==========================================
// 5. 性能测试驱动 
// ==========================================
SortPerformance testAlgorithm(void (*sortFunc)(int*, int), int rawData[], int n, char* name) {
    int* arr = (int*)malloc(n * sizeof(int));
    copyArray(rawData, arr, n);

    g_compareCount = 0;
    g_moveCount = 0;

    // 【修改点】使用高精度计时
    long long start = getTick();
    sortFunc(arr, n);
    long long end = getTick();

    SortPerformance sp;
    strcpy_s(sp.algorithm, sizeof(sp.algorithm), name);
    sp.dataSize = n;
    sp.compareCount = g_compareCount;
    sp.moveCount = g_moveCount;
    sp.timeCost = calcDuration(start, end);

    free(arr);
    return sp;
}

// ==========================================
// 6. 主程序
// ==========================================
int main() {
    int n = 15; // 默认规模
    int* rawData = NULL;
    int choice;

    initTimer(); // 初始化计时器
    rawData = (int*)malloc(n * sizeof(int));
    generateData(rawData, n, 0);

    while (1) {
        printf("\n============================================\n");
        printf("   可视化排序分析系统 v2.0 (全功能版)\n");
        printf("============================================\n");
        printf("1. 设置数据规模并生成随机数 (当前: %d)\n", n);
        printf("2. 导入测试用例 (从文件)\n");
        printf("3. 切换可视化 (当前: %s)\n", g_visualize ? "开启" : "关闭");
        printf("4. 单算法详细测试 (含所有新算法)\n");
        printf("5. 全算法对比并导出报告\n");
        printf("0. 退出\n");
        printf("--------------------------------------------\n");
        printf("请输入: ");
        scanf_s("%d", &choice);

        switch (choice) {
        case 1: {
            printf("输入规模: ");
            scanf_s("%d", &n);
            if (rawData) free(rawData);
            rawData = (int*)malloc(n * sizeof(int));

            printf("分布 (0-随机 1-有序 2-逆序): ");
            int type; scanf_s("%d", &type);
            generateData(rawData, n, type);
            printf("数据已生成。\n");
            break;
        }
        case 2: {
            int newN;
            int* imported = importFromFile(&newN);
            if (imported) {
                if (rawData) free(rawData);
                rawData = imported;
                n = newN;
            }
            break;
        }
        case 3:
            g_visualize = !g_visualize;
            printf("设置已更新。\n");
            break;
        case 4: {
            printf("算法: 1.冒泡 2.插入 3.选择 4.快速 5.希尔 6.堆 7.归并: ");
            int algo; scanf_s("%d", &algo);
            SortPerformance sp;
            memset(&sp, 0, sizeof(sp)); // 安全初始化

            if (algo == 1) sp = testAlgorithm(BubbleSort, rawData, n, "Bubble");
            else if (algo == 2) sp = testAlgorithm(InsertSort, rawData, n, "Insert");
            else if (algo == 3) sp = testAlgorithm(SelectSort, rawData, n, "Selection");
            else if (algo == 4) sp = testAlgorithm(QuickSort, rawData, n, "Quick");
            else if (algo == 5) sp = testAlgorithm(ShellSort, rawData, n, "Shell");
            else if (algo == 6) sp = testAlgorithm(HeapSort, rawData, n, "Heap");
            else if (algo == 7) sp = testAlgorithm(MergeSort, rawData, n, "Merge");
            else { printf("无效选择\n"); break; }

            printf("\n--- 结果: %s ---\n比较: %ld\n移动: %ld\n耗时: %.8f 秒\n",
                sp.algorithm, sp.compareCount, sp.moveCount, sp.timeCost);
            break;
        }
        case 5: {
            int oldVis = g_visualize;
            g_visualize = 0; // 强制关闭可视化以测速
            printf("正在对比7种算法 (规模: %d)...\n", n);

            SortPerformance results[7];
            results[0] = testAlgorithm(BubbleSort, rawData, n, "Bubble");
            results[1] = testAlgorithm(InsertSort, rawData, n, "Insert");
            results[2] = testAlgorithm(SelectSort, rawData, n, "Select");
            results[3] = testAlgorithm(QuickSort, rawData, n, "Quick");
            results[4] = testAlgorithm(ShellSort, rawData, n, "Shell");
            results[5] = testAlgorithm(HeapSort, rawData, n, "Heap");
            results[6] = testAlgorithm(MergeSort, rawData, n, "Merge");

            printf("\n%-10s %-10s %-12s %-12s %-15s\n", "Algo", "Size", "Compares", "Moves", "Time(s)");
            printf("-------------------------------------------------------------\n");
            for (int i = 0; i < 7; i++) {
                printf("%-10s %-10d %-12ld %-12ld %-15.8f\n",
                    results[i].algorithm, results[i].dataSize,
                    results[i].compareCount, results[i].moveCount, results[i].timeCost);
            }

            saveResultsToFile(results, 7); // 自动导出
            g_visualize = oldVis;
            system("pause");
            break;
        }
        case 0:
            free(rawData);
            return 0;
        default: printf("无效输入\n");
        }
    }
}