/*
 * 文件名: sort_dll.c
 * 描述: 核心算法实现与性能统计逻辑
 * 对应文档: 项目需求分析报告 - 模块1 & 模块3
 */

#include "sort_system.h"
#include <stdlib.h>
#include <time.h>
#include <windows.h> // 用于高精度计时

 // 全局统计变量 (每次排序前需重置)
static long long g_comparisons = 0;
static long long g_moves = 0;

// ==========================================
// 内部辅助函数 (不对外导出)
// ==========================================

// 重置统计器
void reset_stats() {
    g_comparisons = 0;
    g_moves = 0;
}

// 交换函数 (移动次数+3)
void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
    g_moves += 3;
}

// 高精度计时包装器
// func: 排序函数指针, arr: 数组, n: 长度
SortStats run_with_timer(void (*func)(int*, int), int* arr, int n) {
    SortStats stats;
    reset_stats();

    LARGE_INTEGER frequency, start, end;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);

    // 执行具体的排序逻辑
    func(arr, n);

    QueryPerformanceCounter(&end);

    // 填充统计结果
    stats.time_ms = (double)(end.QuadPart - start.QuadPart) * 1000.0 / frequency.QuadPart;
    stats.comparisons = g_comparisons;
    stats.moves = g_moves;

    return stats;
}

// ==========================================
// 模块2：数据生成实现
// ==========================================
EXPORT void generate_data(int* arr, int n, int range_max) {
    srand((unsigned int)time(NULL));
    for (int i = 0; i < n; i++) {
        arr[i] = rand() % range_max;
    }
}

// ==========================================
// 模块1：算法核心实现
// ==========================================

// --- 1. 冒泡排序逻辑 ---
void _bubble_impl(int* arr, int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            g_comparisons++;
            if (arr[j] > arr[j + 1]) {
                swap(&arr[j], &arr[j + 1]);
            }
        }
    }
}
// 导出包装
EXPORT SortStats bubble_sort(int* arr, int n) {
    return run_with_timer(_bubble_impl, arr, n);
}

// --- 2. 插入排序逻辑 ---
void _insertion_impl(int* arr, int n) {
    for (int i = 1; i < n; i++) {
        int key = arr[i];
        int j = i - 1;
        g_moves++; // key = arr[i]

        while (j >= 0) {
            g_comparisons++;
            if (arr[j] > key) {
                arr[j + 1] = arr[j];
                g_moves++;
                j--;
            }
            else {
                break;
            }
        }
        arr[j + 1] = key;
        g_moves++;
    }
}
// 导出包装
EXPORT SortStats insertion_sort(int* arr, int n) {
    return run_with_timer(_insertion_impl, arr, n);
}

// --- 3. 选择排序逻辑 ---
void _selection_impl(int* arr, int n) {
    for (int i = 0; i < n - 1; i++) {
        int min_idx = i;
        for (int j = i + 1; j < n; j++) {
            g_comparisons++;
            if (arr[j] < arr[min_idx]) {
                min_idx = j;
            }
        }
        if (min_idx != i) {
            swap(&arr[i], &arr[min_idx]);
        }
    }
}
// 导出包装
EXPORT SortStats selection_sort(int* arr, int n) {
    return run_with_timer(_selection_impl, arr, n);
}

// --- 4. 希尔排序逻辑 ---
void _shell_impl(int* arr, int n) {
    for (int gap = n / 2; gap > 0; gap /= 2) {
        for (int i = gap; i < n; i++) {
            int temp = arr[i];
            g_moves++;
            int j;
            for (j = i; j >= gap; j -= gap) {
                g_comparisons++;
                if (arr[j - gap] > temp) {
                    arr[j] = arr[j - gap];
                    g_moves++;
                }
                else {
                    break;
                }
            }
            arr[j] = temp;
            g_moves++;
        }
    }
}
// 导出包装
EXPORT SortStats shell_sort(int* arr, int n) {
    return run_with_timer(_shell_impl, arr, n);
}

// --- 5. 快速排序逻辑 ---
int _partition(int* arr, int low, int high) {
    int pivot = arr[high];
    g_moves++;
    int i = (low - 1);
    for (int j = low; j <= high - 1; j++) {
        g_comparisons++;
        if (arr[j] < pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

void _quick_recursive(int* arr, int low, int high) {
    if (low < high) {
        int pi = _partition(arr, low, high);
        _quick_recursive(arr, low, pi - 1);
        _quick_recursive(arr, pi + 1, high);
    }
}

// 内部包装：适配 run_with_timer 的函数签名
void _quick_wrapper(int* arr, int n) {
    _quick_recursive(arr, 0, n - 1);
}

// 导出包装
EXPORT SortStats quick_sort(int* arr, int n) {
    return run_with_timer(_quick_wrapper, arr, n);
}

// 6. 归并排序 (Merge Sort)
// 合并两个子数组
void _merge(int arr[], int l, int m, int r, int temp[]) {
    int i = l;
    int j = m + 1;
    int k = l;

    while (i <= m && j <= r) {
        g_comparisons++;
        if (arr[i] <= arr[j]) {
            temp[k++] = arr[i++];
            g_moves++; // 移动到临时数组
        }
        else {
            temp[k++] = arr[j++];
            g_moves++;
        }
    }

    while (i <= m) {
        temp[k++] = arr[i++];
        g_moves++;
    }

    while (j <= r) {
        temp[k++] = arr[j++];
        g_moves++;
    }

    // 将排序好的部分拷贝回原数组
    for (i = l; i <= r; i++) {
        arr[i] = temp[i];
        g_moves++;
    }
}

// 递归分割
void _merge_sort_recursive(int arr[], int l, int r, int temp[]) {
    if (l < r) {
        int m = l + (r - l) / 2;
        _merge_sort_recursive(arr, l, m, temp);
        _merge_sort_recursive(arr, m + 1, r, temp);
        _merge(arr, l, m, r, temp);
    }
}

// 包装器：处理内存分配
void _merge_wrapper(int* arr, int n) {
    int* temp = (int*)malloc(n * sizeof(int));
    if (temp) {
        _merge_sort_recursive(arr, 0, n - 1, temp);
        free(temp);
    }
}

EXPORT SortStats merge_sort(int* arr, int n) {
    return run_with_timer(_merge_wrapper, arr, n);
}

// 7. 堆排序 (Heap Sort)
// 调整堆 (Heapify)
void _heapify(int arr[], int n, int i) {
    int largest = i;
    int l = 2 * i + 1;
    int r = 2 * i + 2;

    // 比较左孩子
    if (l < n) {
        g_comparisons++;
        if (arr[l] > arr[largest])
            largest = l;
    }

    // 比较右孩子
    if (r < n) {
        g_comparisons++;
        if (arr[r] > arr[largest])
            largest = r;
    }

    if (largest != i) {
        swap(&arr[i], &arr[largest]); // swap内部会自动增加 g_moves
        _heapify(arr, n, largest);
    }
}

void _heap_sort_impl(int* arr, int n) {
    // 1. 建堆
    for (int i = n / 2 - 1; i >= 0; i--)
        _heapify(arr, n, i);

    // 2. 排序
    for (int i = n - 1; i > 0; i--) {
        swap(&arr[0], &arr[i]); // 将堆顶元素(最大)移到末尾
        _heapify(arr, i, 0);    // 重新调整剩余部分
    }
}

EXPORT SortStats heap_sort(int* arr, int n) {
    return run_with_timer(_heap_sort_impl, arr, n);
}