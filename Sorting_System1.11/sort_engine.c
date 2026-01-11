#include "sort_engine.h"
#include <stdlib.h>
#include <string.h>
#include <windows.h> 

// 全局统计变量
static long long g_comparisons = 0;
static long long g_moves = 0;
static LARGE_INTEGER g_cpuFreq;

// 0. 基础数据结构：
typedef struct {
    int low;
    int high;
} StackNode;

typedef struct {
    StackNode* data;
    int top;
    int capacity;
} SeqStack;

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
    if (s->data) free(s->data);
}

// 辅助函数
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

int min_val(int x, int y) {
    return x < y ? x : y;
}

// 通用执行器
SortPerformance run_sort(void (*func)(int*, int), int* arr, int n, char* name) {
    SortPerformance sp;
    memset(&sp, 0, sizeof(SortPerformance));
    strncpy_s(sp.algorithm, 30, name, _TRUNCATE);
    sp.dataSize = n;

    reset_stats();
    QueryPerformanceFrequency(&g_cpuFreq);

    LARGE_INTEGER start, end;
    QueryPerformanceCounter(&start);
    func(arr, n);
    QueryPerformanceCounter(&end);

    sp.compareCount = g_comparisons;
    sp.moveCount = g_moves;
    sp.timeCost = (double)(end.QuadPart - start.QuadPart) * 1000.0 / g_cpuFreq.QuadPart;

    return sp;
}

// 数据生成
EXPORT void generate_data_c(int* arr, int n, int type) {
    srand((unsigned)GetTickCount64());
    for (int i = 0; i < n; i++) arr[i] = rand() % 10000;
    if (type == 1) { // 正序
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

// 算法实现

// 1. 冒泡排序
void _BubbleSort(int arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1 - i; j++) {
            g_comparisons++;
            if (arr[j] > arr[j + 1]) swap(&arr[j], &arr[j + 1]);
        }
    }
}

// 2. 插入排序
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

// 3. 选择排序
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

// 4. 希尔排序
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
void _HeapAdjustIterative(int arr[], int n, int i) {
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
    for (int i = n / 2 - 1; i >= 0; i--) _HeapAdjustIterative(arr, n, i);
    for (int i = n - 1; i > 0; i--) {
        swap(&arr[0], &arr[i]);
        _HeapAdjustIterative(arr, i, 0);
    }
}

// 6. 归并排序
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
void _MergeSortIterative(int arr[], int n) {
    int* temp = (int*)malloc(n * sizeof(int));
    if (!temp) return;

    for (int curr_size = 1; curr_size <= n - 1; curr_size = 2 * curr_size) {
        for (int left_start = 0; left_start < n - 1; left_start += 2 * curr_size) {
            int mid = min_val(left_start + curr_size - 1, n - 1);
            int right_end = min_val(left_start + 2 * curr_size - 1, n - 1);
            _Merge(arr, left_start, mid, right_end, temp);
        }
    }
    free(temp);
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
void _QuickSortIterative(int arr[], int n) {
    if (n <= 1) return;

    SeqStack stack;
    initStack(&stack, n); // 初始化栈
    push(&stack, 0, n - 1); // 压入初始区间

    while (!isStackEmpty(&stack)) {
        int low, high;
        pop(&stack, &low, &high);

        int pivot = _Partition(arr, low, high);

        if (pivot + 1 < high) push(&stack, pivot + 1, high);
        if (low < pivot - 1) push(&stack, low, pivot - 1);
    }
    freeStack(&stack);
}

// 导出
EXPORT SortPerformance bubble_sort(int* arr, int n) { return run_sort(_BubbleSort, arr, n, "Bubble Sort"); }
EXPORT SortPerformance insertion_sort(int* arr, int n) { return run_sort(_InsertSort, arr, n, "Insertion Sort"); }
EXPORT SortPerformance selection_sort(int* arr, int n) { return run_sort(_SelectSort, arr, n, "Selection Sort"); }
EXPORT SortPerformance shell_sort(int* arr, int n) { return run_sort(_ShellSort, arr, n, "Shell Sort"); }
EXPORT SortPerformance quick_sort(int* arr, int n) { return run_sort(_QuickSortIterative, arr, n, "Quick Sort"); }
EXPORT SortPerformance merge_sort(int* arr, int n) { return run_sort(_MergeSortIterative, arr, n, "Merge Sort"); }
EXPORT SortPerformance heap_sort(int* arr, int n) { return run_sort(_HeapSort, arr, n, "Heap Sort"); }