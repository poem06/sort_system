#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <windows.h> // 用于Sleep和清屏

// ==========================================
// 1. 数据结构定义 
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
int g_visualize = 0;       // 可视化开关 1=开, 0=关 
int g_delay = 50;          // 可视化延时(ms)

// ==========================================
// 2. 辅助函数：可视化与数据生成 
// ==========================================

// 可视化展示函数
void visualize(int arr[], int n) {
    if (!g_visualize) return;

    system("cls");
    printf("--- 排序可视化中 ---\n");
    printf("当前数据状态:\n");

    for (int i = 0; i < n; i++) {
        printf("%3d | ", arr[i]);
        if (n <= 20) {
            for (int k = 0; k < arr[i]; k++) printf("*");
        }
        printf("\n");
    }
    Sleep(g_delay);
}

// 交换两个元素
void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
    g_moveCount += 3;
}

// 生成随机数据 
void generateData(int arr[], int n, int type) {
    srand((unsigned)time(NULL));
    for (int i = 0; i < n; i++) {
        arr[i] = rand() % 50 + 1;
    }

    if (type == 1) {
        // 构造有序 
        for (int i = 0;i < n - 1;i++) for (int j = 0;j < n - 1 - i;j++) if (arr[j] > arr[j + 1]) {
            int t = arr[j]; arr[j] = arr[j + 1]; arr[j + 1] = t;
        }
    }
    else if (type == 2) {
        // 构造逆序
        for (int i = 0;i < n - 1;i++) for (int j = 0;j < n - 1 - i;j++) if (arr[j] < arr[j + 1]) {
            int t = arr[j]; arr[j] = arr[j + 1]; arr[j + 1] = t;
        }
    }
}

// 复制数组
void copyArray(int src[], int dest[], int n) {
    for (int i = 0; i < n; i++) dest[i] = src[i];
}

// ==========================================
// 3. 排序算法实现 
// ==========================================

// 3.1 冒泡排序 
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

// 3.2 直接插入排序 
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

// 3.3 直接选择排序 
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

// 3.4 快速排序 (递归版) 
int Partition(int arr[], int low, int high, int n_visual) {
    int pivot = arr[low];
    g_moveCount++;

    while (low < high) {
        while (low < high && arr[high] >= pivot) {
            g_compareCount++;
            high--;
        }
        g_compareCount++;

        arr[low] = arr[high];
        g_moveCount++;

        while (low < high && arr[low] <= pivot) {
            g_compareCount++;
            low++;
        }
        g_compareCount++;

        arr[high] = arr[low];
        g_moveCount++;
        visualize(arr, n_visual);
    }
    arr[low] = pivot;
    g_moveCount++;
    return low;
}

void QuickSort(int arr[], int low, int high, int n_visual) {
    if (low < high) {
        int pivotPos = Partition(arr, low, high, n_visual);
        QuickSort(arr, low, pivotPos - 1, n_visual);
        QuickSort(arr, pivotPos + 1, high, n_visual);
    }
}

// ==========================================
// 4. 性能测试驱动 
// ==========================================
SortPerformance testAlgorithm(void (*sortFunc)(int*, int), int rawData[], int n, char* name) {
    int* arr = (int*)malloc(n * sizeof(int));
    copyArray(rawData, arr, n);

    g_compareCount = 0;
    g_moveCount = 0;

    clock_t start = clock();
    sortFunc(arr, n);
    clock_t end = clock();

    SortPerformance sp;
    // 【修改点】使用 strcpy_s，并指定目标缓冲区大小
    strcpy_s(sp.algorithm, sizeof(sp.algorithm), name);
    sp.dataSize = n;
    sp.compareCount = g_compareCount;
    sp.moveCount = g_moveCount;
    sp.timeCost = (double)(end - start) / CLOCKS_PER_SEC;

    free(arr);
    return sp;
}

SortPerformance testQuickSort(int rawData[], int n) {
    int* arr = (int*)malloc(n * sizeof(int));
    copyArray(rawData, arr, n);
    g_compareCount = 0; g_moveCount = 0;

    clock_t start = clock();
    QuickSort(arr, 0, n - 1, n);
    clock_t end = clock();

    SortPerformance sp;
    // 【修改点】使用 strcpy_s
    strcpy_s(sp.algorithm, sizeof(sp.algorithm), "Quick Sort");
    sp.dataSize = n;
    sp.compareCount = g_compareCount;
    sp.moveCount = g_moveCount;
    sp.timeCost = (double)(end - start) / CLOCKS_PER_SEC;

    free(arr);
    return sp;
}

// ==========================================
// 5. 主菜单与交互 
// ==========================================
int main() {
    int n = 10;
    int* rawData = NULL;
    int choice;
    int dataGenerated = 0;

    rawData = (int*)malloc(n * sizeof(int));
    generateData(rawData, n, 0);

    while (1) {
        printf("\n============================================\n");
        printf("   可视化排序算法分析系统 (MSVC 安全版)\n");
        printf("============================================\n");
        printf("1. 设置数据规模 (当前: %d)\n", n);
        printf("2. 生成测试数据 (随机/有序/逆序)\n");
        printf("3. 开启/关闭可视化 (当前: %s)\n", g_visualize ? "开启" : "关闭");
        printf("4. 运行单一算法测试\n");
        printf("5. 运行所有算法对比分析\n");
        printf("0. 退出\n");
        printf("请输入选项: ");

        // 【修改点】使用 scanf_s
        scanf_s("%d", &choice);

        switch (choice) {
        case 1:
            printf("输入新的数据规模 (建议可视化时<20, 测性能时>1000): ");
            // 【修改点】使用 scanf_s
            scanf_s("%d", &n);
            if (rawData) free(rawData);
            rawData = (int*)malloc(n * sizeof(int));
            generateData(rawData, n, 0);
            dataGenerated = 1;
            break;
        case 2:
            printf("选择分布: 0-随机, 1-有序, 2-逆序: ");
            int type;
            // 【修改点】使用 scanf_s
            scanf_s("%d", &type);
            generateData(rawData, n, type);
            printf("数据已重新生成。\n");
            dataGenerated = 1;
            break;
        case 3:
            g_visualize = !g_visualize;
            printf("可视化已%s。\n", g_visualize ? "开启" : "关闭");
            break;
        case 4: {
            if (!dataGenerated) generateData(rawData, n, 0);
            printf("1.冒泡 2.插入 3.选择 4.快速: ");
            int algo;
            // 【修改点】使用 scanf_s
            scanf_s("%d", &algo);
            SortPerformance sp;
            // 这里需要初始化 sp 以防编译器警告，虽然下面的逻辑会覆盖它
            memset(&sp, 0, sizeof(SortPerformance));

            if (algo == 1) sp = testAlgorithm(BubbleSort, rawData, n, "Bubble Sort");
            else if (algo == 2) sp = testAlgorithm(InsertSort, rawData, n, "Insert Sort");
            else if (algo == 3) sp = testAlgorithm(SelectSort, rawData, n, "Select Sort");
            else if (algo == 4) sp = testQuickSort(rawData, n);

            printf("\n--- 测试结果 ---\n");
            printf("算法: %s\n比较次数: %ld\n移动次数: %ld\n耗时: %.6f 秒\n",
                sp.algorithm, sp.compareCount, sp.moveCount, sp.timeCost);
            system("pause");
            break;
        }
        case 5: {
            int oldVis = g_visualize;
            g_visualize = 0;
            printf("正在进行全算法对比 (可视化已临时关闭)...\n");

            SortPerformance p1 = testAlgorithm(BubbleSort, rawData, n, "Bubble Sort");
            SortPerformance p2 = testAlgorithm(InsertSort, rawData, n, "Insert Sort");
            SortPerformance p3 = testAlgorithm(SelectSort, rawData, n, "Select Sort");
            SortPerformance p4 = testQuickSort(rawData, n);

            printf("\n%-15s %-10s %-15s %-15s %-10s\n", "Algorithm", "Size", "Comparisons", "Moves", "Time(s)");
            printf("-------------------------------------------------------------------\n");
            printf("%-15s %-10d %-15ld %-15ld %-10.6f\n", p1.algorithm, p1.dataSize, p1.compareCount, p1.moveCount, p1.timeCost);
            printf("%-15s %-10d %-15ld %-15ld %-10.6f\n", p2.algorithm, p2.dataSize, p2.compareCount, p2.moveCount, p2.timeCost);
            printf("%-15s %-10d %-15ld %-15ld %-10.6f\n", p3.algorithm, p3.dataSize, p3.compareCount, p3.moveCount, p3.timeCost);
            printf("%-15s %-10d %-15ld %-15ld %-10.6f\n", p4.algorithm, p4.dataSize, p4.compareCount, p4.moveCount, p4.timeCost);

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