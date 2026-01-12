#ifndef SORT_ENGINE_H
#define SORT_ENGINE_H

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

#pragma pack(push, 1)
typedef struct {
    char algorithm[30];   
    int dataSize;         
    long long compareCount;
    long long moveCount;   
    double timeCost;       
} SortPerformance;
#pragma pack(pop)

EXPORT SortPerformance bubble_sort(int* arr, int n);
EXPORT SortPerformance insertion_sort(int* arr, int n);
EXPORT SortPerformance selection_sort(int* arr, int n);
EXPORT SortPerformance shell_sort(int* arr, int n);
EXPORT SortPerformance quick_sort(int* arr, int n);
EXPORT SortPerformance merge_sort(int* arr, int n);
EXPORT SortPerformance heap_sort(int* arr, int n);

// 数据生成接口
EXPORT void generate_data_c(int* arr, int n, int type);

#endif // SORT_ENGINE_H