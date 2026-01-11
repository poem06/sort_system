#include "sort_engine.h"
#include <stdlib.h>
#include <string.h>
#include <windows.h> 

// 全局统计变量
static long long g_comparisons = 0;
static long long g_moves = 0;
static LARGE_INTEGER g_cpuFreq;

void reset_stats() {
    g_comparisons = 0;
    g_moves = 0;
}

void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
    g_moves += 3;
}

// 通用执行器
SortPerformance run_sort(void (*func)(int*, int), int* arr, int n, char* name) {
    SortPerformance sp;
    // 初始化结构体，防止垃圾值
    memset(&sp, 0, sizeof(SortPerformance));

    strncpy_s(sp.algorithm, 30, name, _TRUNCATE);
    sp.dataSize = n;

    reset_stats();
    QueryPerformanceFrequency(&g_cpuFreq);

    LARGE_INTEGER start, end;
    QueryPerformanceCounter(&start);
    func(arr, n); // 执行排序
    QueryPerformanceCounter(&end);

    sp.compareCount = g_comparisons;
    sp.moveCount = g_moves;
    // 转换为毫秒(ms)
    sp.timeCost = (double)(end.QuadPart - start.QuadPart) * 1000.0 / g_cpuFreq.QuadPart;

    return sp;
}

// 数据生成
// type: 0=随机, 1=正序, 2=逆序
EXPORT void generate_data_c(int* arr, int n, int type) {
    srand((unsigned)GetTickCount64());
    for (int i = 0; i < n; i++) {
        arr[i] = rand() % 10000; // 随机
    }

    if (type == 1) { // 正序
        // 简单冒泡排成正序
        for (int i = 0; i < n - 1; i++)
            for (int j = 0; j < n - 1 - i; j++)
                if (arr[j] > arr[j + 1]) { int t = arr[j]; arr[j] = arr[j + 1]; arr[j + 1] = t; }
    }
    else if (type == 2) { // 逆序
        for (int i = 0; i < n - 1; i++)
            for (int j = 0; j < n - 1 - i; j++)
                if (arr[j] < arr[j + 1]) { int t = arr[j]; arr[j] = arr[j + 1]; arr[j + 1] = t; }
    }
}

// 1. 冒泡
void _BubbleSort(int arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1 - i; j++) {
            g_comparisons++;
            if (arr[j] > arr[j + 1]) swap(&arr[j], &arr[j + 1]);
        }
    }
}

// 2. 插入
void _InsertSort(int arr[], int n) {
    for (int i = 1; i < n; i++) {
        int temp = arr[i]; g_moves++;
        int j = i - 1;
        while (j >= 0) {
            g_comparisons++;
            if (arr[j] > temp) {
                arr[j + 1] = arr[j]; g_moves++;
                j--;
            }
            else break;
        }
        arr[j + 1] = temp; g_moves++;
    }
}

// 3. 选择
void _SelectSort(int arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        int minIdx = i;
        for (int j = i + 1; j < n; j++) {
            g_comparisons++;
            if (arr[j] < arr[minIdx]) minIdx = j;
        }
        if (minIdx != i) swap(&arr[i], &arr[minIdx]);
    }
}

// 4. 希尔
void _ShellSort(int arr[], int n) {
    for (int gap = n / 2; gap > 0; gap /= 2) {
        for (int i = gap; i < n; i++) {
            int temp = arr[i]; g_moves++;
            int j = i;
            while (j >= gap) {
                g_comparisons++;
                if (arr[j - gap] > temp) {
                    arr[j] = arr[j - gap]; g_moves++;
                    j -= gap;
                }
                else break;
            }
            arr[j] = temp; g_moves++;
        }
    }
}

// 5. 堆排序
void _HeapAdjust(int arr[], int n, int i) {
    int temp = arr[i]; g_moves++;
    int k = 2 * i + 1;
    while (k < n) {
        if (k + 1 < n) {
            g_comparisons++;
            if (arr[k] < arr[k + 1]) k++;
        }
        g_comparisons++;
        if (arr[k] > temp) {
            arr[i] = arr[k]; g_moves++;
            i = k;
            k = 2 * i + 1;
        }
        else break;
    }
    arr[i] = temp; g_moves++;
}
void _HeapSort(int arr[], int n) {
    for (int i = n / 2 - 1; i >= 0; i--) _HeapAdjust(arr, n, i);
    for (int i = n - 1; i > 0; i--) {
        swap(&arr[0], &arr[i]);
        _HeapAdjust(arr, i, 0);
    }
}

// 6. 归并
void _Merge(int arr[], int l, int m, int r, int temp[]) {
    int i = l, j = m + 1, k = l;
    while (i <= m && j <= r) {
        g_comparisons++;
        if (arr[i] <= arr[j]) { temp[k++] = arr[i++]; g_moves++; }
        else { temp[k++] = arr[j++]; g_moves++; }
    }
    while (i <= m) { temp[k++] = arr[i++]; g_moves++; }
    while (j <= r) { temp[k++] = arr[j++]; g_moves++; }
    for (i = l; i <= r; i++) { arr[i] = temp[i]; g_moves++; }
}
void _MergeSortRecursive(int arr[], int l, int r, int temp[]) {
    if (l < r) {
        int m = l + (r - l) / 2;
        _MergeSortRecursive(arr, l, m, temp);
        _MergeSortRecursive(arr, m + 1, r, temp);
        _Merge(arr, l, m, r, temp);
    }
}
void _MergeSort(int arr[], int n) {
    int* temp = (int*)malloc(n * sizeof(int));
    if (temp) {
        _MergeSortRecursive(arr, 0, n - 1, temp);
        free(temp);
    }
}

// 7. 快速排序
int _Partition(int arr[], int low, int high) {
    int pivot = arr[high]; g_moves++;
    int i = low - 1;
    for (int j = low; j < high; j++) {
        g_comparisons++;
        if (arr[j] < pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return i + 1;
}
void _QuickSortRecursive(int arr[], int low, int high) {
    if (low < high) {
        int pi = _Partition(arr, low, high);
        _QuickSortRecursive(arr, low, pi - 1);
        _QuickSortRecursive(arr, pi + 1, high);
    }
}
void _QuickSort(int arr[], int n) {
    _QuickSortRecursive(arr, 0, n - 1);
}

// --- 导出实现 ---
EXPORT SortPerformance bubble_sort(int* arr, int n) { return run_sort(_BubbleSort, arr, n, "Bubble Sort"); }
EXPORT SortPerformance insertion_sort(int* arr, int n) { return run_sort(_InsertSort, arr, n, "Insertion Sort"); }
EXPORT SortPerformance selection_sort(int* arr, int n) { return run_sort(_SelectSort, arr, n, "Selection Sort"); }
EXPORT SortPerformance shell_sort(int* arr, int n) { return run_sort(_ShellSort, arr, n, "Shell Sort"); }
EXPORT SortPerformance quick_sort(int* arr, int n) { return run_sort(_QuickSort, arr, n, "Quick Sort"); }
EXPORT SortPerformance merge_sort(int* arr, int n) { return run_sort(_MergeSort, arr, n, "Merge Sort"); }
EXPORT SortPerformance heap_sort(int* arr, int n) { return run_sort(_HeapSort, arr, n, "Heap Sort"); }