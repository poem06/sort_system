/*
 * 文件名: sort_dll.h
 * 描述: 定义导出宏、性能统计结构体及算法接口
 * 对应文档: 项目需求分析报告 - 模块1 & 模块3
 */

#ifndef SORT_DLL_H
#define SORT_DLL_H

 // Windows DLL 导出宏定义
#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

// 性能统计结构体 (对应需求：技术选型-核心数据结构)
// 用于将 C 语言统计的指标一次性传回 Python
typedef struct {
    long long comparisons; // 比较次数
    long long moves;       // 移动次数
    double time_ms;        // 执行耗时(毫秒)
} SortStats;

// --- 模块2：数据生成 ---
EXPORT void generate_data(int* arr, int n, int range_max);

// --- 模块1：算法实现 (全部封装为返回 SortStats) ---
EXPORT SortStats bubble_sort(int* arr, int n);
EXPORT SortStats insertion_sort(int* arr, int n);
EXPORT SortStats selection_sort(int* arr, int n);
EXPORT SortStats shell_sort(int* arr, int n);
EXPORT SortStats quick_sort(int* arr, int n);
EXPORT SortStats merge_sort(int* arr, int n);
EXPORT SortStats heap_sort(int* arr, int n);

#endif // SORT_DLL_H