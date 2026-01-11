#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <windows.h> // 用于Sleep, 清屏, 高精度计时


 // 1. 数据结构定义 

 // 性能统计结构体 
typedef struct SortPerformance {
    char algorithm[20];    // 算法名称
    int dataSize;          // 数据规模
    long compareCount;     // 比较次数
    long moveCount;        // 移动次数
    double timeCost;       // 执行耗时（秒）
} SortPerformance;

// 栈结构
typedef struct {
    int low;
    int high;
} StackNode;

typedef struct {
    StackNode* data;       // 动态数组
    int top;               // 栈顶指针
    int capacity;          // 栈容量
} SeqStack;

// 全局变量
long g_compareCount = 0;
long g_moveCount = 0;
int g_visualize = 0;       // 可视化开关 
int g_delay = 50;          // 动画延时(ms)
LARGE_INTEGER g_cpuFreq;   // CPU计时频率

// 2. 栈操作函数

void initStack(SeqStack* s, int capacity) {
    s->data = (StackNode*)malloc(capacity * sizeof(StackNode));
    s->top = -1;
    s->capacity = capacity;
}

int isStackEmpty(SeqStack* s) {
    return s->top == -1;
}

void push(SeqStack* s, int low, int high) {
    if (s->top < s->capacity - 1) {
        s->top++;
        s->data[s->top].low = low;
        s->data[s->top].high = high;
    }
}

void pop(SeqStack* s, int* low, int* high) {
    if (s->top >= 0) {
        *low = s->data[s->top].low;
        *high = s->data[s->top].high;
        s->top--;
    }
}

void freeStack(SeqStack* s) {
    if (s->data) {
        free(s->data);
        s->data = NULL;
    }
}

// 3. 基础辅助函数

// 初始化高精度计时器
void initTimer() {
    QueryPerformanceFrequency(&g_cpuFreq);
}

// 获取当前微秒级时间戳
long long getTick() {
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);
    return t.QuadPart;
}

// 计算时间差(秒)
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
        // 简单柱状图
        if (n <= 30) {
            for (int k = 0; k < arr[i]; k++) printf("*");
        }
        printf("\n");
    }
    Sleep(g_delay);
}

// 交换元素
void swap(int* a, int* b) {
    int temp = *a; *a = *b; *b = temp;
    g_moveCount += 3; // 交换计为3次移动 
}

// 数组拷贝
void copyArray(int src[], int dest[], int n) {
    for (int i = 0; i < n; i++) dest[i] = src[i];
}

// 辅助函数：取最小值
int min_val(int x, int y) {
    return x < y ? x : y;
}

// 4. 排序算法实现 (全非递归) 

// 4.1 冒泡排序
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

// 4.2 直接插入排序
void InsertSort(int arr[], int n) {
    int i, j, temp;
    for (i = 1; i < n; i++) {
        temp = arr[i]; g_moveCount++;
        j = i - 1; g_compareCount++;
        while (j >= 0 && arr[j] > temp) {
            g_compareCount++;
            arr[j + 1] = arr[j]; g_moveCount++;
            j--;
            visualize(arr, n);
        }
        arr[j + 1] = temp; g_moveCount++;
        visualize(arr, n);
    }
}

// 4.3 直接选择排序 
void SelectSort(int arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        int minIdx = i;
        for (int j = i + 1; j < n; j++) {
            g_compareCount++;
            if (arr[j] < arr[minIdx]) minIdx = j;
        }
        if (minIdx != i) {
            swap(&arr[i], &arr[minIdx]);
            visualize(arr, n);
        }
    }
}

// 4.4 希尔排序
void ShellSort(int arr[], int n) {
    int gap, i, j, temp;
    for (gap = n / 2; gap > 0; gap /= 2) {
        for (i = gap; i < n; i++) {
            temp = arr[i]; g_moveCount++;
            j = i; g_compareCount++;
            while (j >= gap && arr[j - gap] > temp) {
                g_compareCount++;
                arr[j] = arr[j - gap]; g_moveCount++;
                j -= gap;
                visualize(arr, n);
            }
            arr[j] = temp; g_moveCount++;
        }
    }
}

// 4.5 堆排序
// 将原本的递归Heapify改为循环下沉
void HeapAdjustIterative(int arr[], int n, int i) {
    int temp = arr[i]; g_moveCount++;
    int k = 2 * i + 1; // 左孩子

    while (k < n) {
        g_compareCount++;
        // 找最大的孩子
        if (k + 1 < n && arr[k] < arr[k + 1]) {
            k++;
        }

        g_compareCount++;
        if (arr[k] > temp) {
            arr[i] = arr[k]; g_moveCount++;
            i = k; // 继续向下寻找位置
            k = 2 * i + 1;
        }
        else {
            break;
        }
    }
    arr[i] = temp; g_moveCount++;
}

void HeapSort(int arr[], int n) {
    // 1. 建堆
    for (int i = n / 2 - 1; i >= 0; i--) {
        HeapAdjustIterative(arr, n, i);
        visualize(arr, n);
    }
    // 2. 排序
    for (int i = n - 1; i > 0; i--) {
        swap(&arr[0], &arr[i]);
        visualize(arr, n);
        HeapAdjustIterative(arr, i, 0);
        visualize(arr, n);
    }
}

// 4.6 归并排序
void Merge(int arr[], int l, int m, int r, int temp[]) {
    int i = l, j = m + 1, k = l;
    while (i <= m && j <= r) {
        g_compareCount++;
        if (arr[i] <= arr[j]) { temp[k++] = arr[i++]; g_moveCount++; }
        else { temp[k++] = arr[j++]; g_moveCount++; }
    }
    while (i <= m) { temp[k++] = arr[i++]; g_moveCount++; }
    while (j <= r) { temp[k++] = arr[j++]; g_moveCount++; }

    // 拷贝回原数组
    for (i = l; i <= r; i++) { arr[i] = temp[i]; g_moveCount++; }
}

void MergeSort(int arr[], int n) {
    int* temp = (int*)malloc(n * sizeof(int));
    if (!temp) return;

    // curr_size: 当前合并子序列的大小 1 -> 2 -> 4 -> 8...
    for (int curr_size = 1; curr_size <= n - 1; curr_size = 2 * curr_size) {
        // left_start: 当前合并组的起始位置
        for (int left_start = 0; left_start < n - 1; left_start += 2 * curr_size) {
            int mid = min_val(left_start + curr_size - 1, n - 1);
            int right_end = min_val(left_start + 2 * curr_size - 1, n - 1);

            Merge(arr, left_start, mid, right_end, temp);
            visualize(arr, n);
        }
    }
    free(temp);
}

// 4.7 快速排序
int Partition(int arr[], int low, int high, int n_visual) {
    int pivot = arr[low]; g_moveCount++;
    while (low < high) {
        while (low < high && arr[high] >= pivot) { g_compareCount++; high--; }
        g_compareCount++; arr[low] = arr[high]; g_moveCount++;
        while (low < high && arr[low] <= pivot) { g_compareCount++; low++; }
        g_compareCount++; arr[high] = arr[low]; g_moveCount++;
        visualize(arr, n_visual);
    }
    arr[low] = pivot; g_moveCount++;
    return low;
}

void QuickSortIterative(int arr[], int n) {
    if (n <= 1) return;

    SeqStack stack;
    initStack(&stack, n); // 初始化栈
    push(&stack, 0, n - 1); // 压入初始区间

    int low, high, pivot;

    while (!isStackEmpty(&stack)) {
        pop(&stack, &low, &high);

        pivot = Partition(arr, low, high, n);

        // 如果右边有元素，压栈
        if (pivot + 1 < high) {
            push(&stack, pivot + 1, high);
        }
        // 如果左边有元素，压栈
        if (low < pivot - 1) {
            push(&stack, low, pivot - 1);
        }
    }
    freeStack(&stack);
}

// 5. 数据管理与IO模块 
void generateData(int arr[], int n, int type) {
    srand((unsigned)time(NULL));
    for (int i = 0; i < n; i++) arr[i] = rand() % 100 + 1;
    if (type == 1) { // Sorted
        for (int i = 0; i < n - 1; i++) for (int j = 0; j < n - 1 - i; j++)
            if (arr[j] > arr[j + 1]) { int t = arr[j]; arr[j] = arr[j + 1]; arr[j + 1] = t; }
    }
    else if (type == 2) { // Reverse
        for (int i = 0; i < n - 1; i++) for (int j = 0; j < n - 1 - i; j++)
            if (arr[j] < arr[j + 1]) { int t = arr[j]; arr[j] = arr[j + 1]; arr[j + 1] = t; }
    }
}

int* importFromFile(int* n) {
    char filename[100];
    printf("请输入文件名 (例如 data.txt): ");
    scanf_s("%s", filename, (unsigned)sizeof(filename));

    FILE* fp;
    if (fopen_s(&fp, filename, "r") != 0) {
        printf("错误：无法打开文件\n");
        return NULL;
    }
    if (fscanf_s(fp, "%d", n) != 1) {
        fclose(fp);
        return NULL;
    }

    int* arr = (int*)malloc((*n) * sizeof(int));
    for (int i = 0; i < *n; i++) fscanf_s(fp, "%d", &arr[i]);

    fclose(fp);
    printf("成功导入 %d 个数据。\n", *n);
    return arr;
}

// 辅助：获取分布名称
const char* getDistName(int type) {
    if (type == 1) return "Sorted";
    if (type == 2) return "Reverse";
    return "Random";
}

// 导出结果 (带时间戳和数据类型)
void saveResultsToFile(SortPerformance results[], int count, int type) {
    FILE* fp;
    char filename[] = "sort_report.csv";

    // 获取当前系统时间
    time_t rawtime;
    struct tm timeinfo;
    char timeStr[30];
    time(&rawtime);
    localtime_s(&timeinfo, &rawtime);
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);

    // 追加模式打开
    if (fopen_s(&fp, filename, "a") != 0) {
        printf("错误：无法写入文件\n");
        return;
    }

    // 若文件为空，写入表头
    fseek(fp, 0, SEEK_END);
    if (ftell(fp) == 0) {
        fprintf(fp, "BatchTime,DataType,Algorithm,DataSize,Comparisons,Moves,Time(s)\n");
    }

    // 写入数据
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%s,%s,%s,%d,%ld,%ld,%.8f\n",
            timeStr,
            getDistName(type),
            results[i].algorithm,
            results[i].dataSize,
            results[i].compareCount,
            results[i].moveCount,
            results[i].timeCost);
    }

    fclose(fp);
    printf("结果已导出至 %s (含时间戳)\n", filename);
}

// 算法测试驱动
SortPerformance testAlgorithm(void (*sortFunc)(int*, int), int rawData[], int n, char* name) {
    int* arr = (int*)malloc(n * sizeof(int));
    copyArray(rawData, arr, n);

    g_compareCount = 0;
    g_moveCount = 0;

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

// 6. 主程序
int main() {
    int n = 15;
    int* rawData = NULL;
    int choice;
    int type = 0; // 0-Random, 1-Sorted, 2-Reverse

    initTimer();
    rawData = (int*)malloc(n * sizeof(int));
    generateData(rawData, n, type);

    while (1) {
        printf("\n============================================\n");
        printf("   可视化排序系统 (Final: 全非递归+CSV优化)\n");
        printf("============================================\n");
        printf("1. 设置规模/生成数据 (当前: %d, 类型: %s)\n", n, getDistName(type));
        printf("2. 导入测试用例 (文件)\n");
        printf("3. 切换可视化 (当前: %s)\n", g_visualize ? "开启" : "关闭");
        printf("4. 单算法测试\n");
        printf("5. 全算法对比 (自动导出CSV)\n");
        printf("0. 退出\n");
        printf("--------------------------------------------\n");
        printf("请输入: ");
        scanf_s("%d", &choice);

        switch (choice) {
        case 1: {
            printf("输入规模: "); scanf_s("%d", &n);
            if (rawData) free(rawData);
            rawData = (int*)malloc(n * sizeof(int));
            printf("分布 (0-随机 1-有序 2-逆序): ");
            scanf_s("%d", &type); // 更新全局type
            generateData(rawData, n, type);
            break;
        }
        case 2: {
            int newN;
            int* imported = importFromFile(&newN);
            if (imported) {
                if (rawData) free(rawData);
                rawData = imported;
                n = newN;
                type = 0; // 导入数据默认为随机/未知类型
            }
            break;
        }
        case 3:
            g_visualize = !g_visualize;
            break;
        case 4: {
            printf("算法: 1.冒泡 2.插入 3.选择 4.快速(栈) 5.希尔 6.堆(迭代) 7.归并(迭代): ");
            int algo; scanf_s("%d", &algo);
            SortPerformance sp;
            memset(&sp, 0, sizeof(sp));

            if (algo == 1) sp = testAlgorithm(BubbleSort, rawData, n, "Bubble");
            else if (algo == 2) sp = testAlgorithm(InsertSort, rawData, n, "Insert");
            else if (algo == 3) sp = testAlgorithm(SelectSort, rawData, n, "Select");
            else if (algo == 4) sp = testAlgorithm(QuickSortIterative, rawData, n, "Quick(Stack)");
            else if (algo == 5) sp = testAlgorithm(ShellSort, rawData, n, "Shell");
            else if (algo == 6) sp = testAlgorithm(HeapSort, rawData, n, "Heap(Iter)");
            else if (algo == 7) sp = testAlgorithm(MergeSort, rawData, n, "Merge(Iter)");
            else { printf("无效选择\n"); break; }

            printf("\n--- 结果: %s ---\n比较: %ld\n移动: %ld\n耗时: %.8f 秒\n",
                sp.algorithm, sp.compareCount, sp.moveCount, sp.timeCost);
            break;
        }
        case 5: {
            int oldVis = g_visualize;
            g_visualize = 0; // 强制关闭可视化以测速

            printf("正在对比7种算法 (规模: %d, 类型: %s)...\n", n, getDistName(type));
            SortPerformance results[7];

            results[0] = testAlgorithm(BubbleSort, rawData, n, "Bubble");
            results[1] = testAlgorithm(InsertSort, rawData, n, "Insert");
            results[2] = testAlgorithm(SelectSort, rawData, n, "Select");
            results[3] = testAlgorithm(QuickSortIterative, rawData, n, "Quick(Stack)");
            results[4] = testAlgorithm(ShellSort, rawData, n, "Shell");
            results[5] = testAlgorithm(HeapSort, rawData, n, "Heap(Iter)");
            results[6] = testAlgorithm(MergeSort, rawData, n, "Merge(Iter)");

            printf("\n%-15s %-12s %-12s %-15s\n", "Algo", "Compares", "Moves", "Time(s)");
            printf("------------------------------------------------------\n");
            for (int i = 0; i < 7; i++) {
                printf("%-15s %-12ld %-12ld %-15.8f\n",
                    results[i].algorithm, results[i].compareCount,
                    results[i].moveCount, results[i].timeCost);
            }

            // 导出时传入 type
            saveResultsToFile(results, 7, type);

            g_visualize = oldVis;
            system("pause");
            break;
        }
        case 0:
            free(rawData);
            return 0;
        default:
            printf("无效输入\n");
        }
    }
}