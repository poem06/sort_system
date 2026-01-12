#ifndef HEAD_H
#define HEAD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

// 1. 数据结构定义

// 排序性能统计结构体
typedef struct SortPerformance {
    char algorithm[20];   // 算法名称
    int dataSize;         // 数据规模
    long compareCount;    // 比较次数
    long moveCount;       // 移动次数
    double timeCost;      // 执行耗时 (秒)
} SortPerformance;

// 非递归快速排序用的栈节点
typedef struct {
    int low;
    int high;
} StackNode;

// 顺序栈结构
typedef struct {
    StackNode* data;      // 动态数组
    int top;              // 栈顶指针
    int capacity;         // 栈容量
} SeqStack;

// 2. 全局变量定义

extern long g_compareCount;
extern long g_moveCount;
extern int g_visualize;          // 可视化开关
extern int g_delay;             // 动画延时(ms)
extern LARGE_INTEGER g_cpuFreq;      // CPU计时频率

// 3. 函数声明

// 栈操作函数
void initStack(SeqStack* s, int capacity);
int isStackEmpty(SeqStack* s);
void push(SeqStack* s, int low, int high);
void pop(SeqStack* s, int* low, int* high);
void freeStack(SeqStack* s);

// 基础辅助函数
void initTimer();
long long getTick();
double calcDuration(long long start, long long end);
void visualize(int arr[], int n);
void swap(int* a, int* b);
void copyArray(int src[], int dest[], int n);
int min_val(int x, int y);

// 排序算法
void BubbleSort(int arr[], int n);
void InsertSort(int arr[], int n);
void SelectSort(int arr[], int n);
void ShellSort(int arr[], int n);

// 堆排序相关
void HeapAdjustIterative(int arr[], int n, int i);
void HeapSort(int arr[], int n);

// 归并排序相关
void Merge(int arr[], int l, int m, int r, int temp[]);
void MergeSort(int arr[], int n);

// 快速排序相关
int Partition(int arr[], int low, int high, int n_visual);
void QuickSortIterative(int arr[], int n);

// 数据管理与IO模块
void generateData(int arr[], int n, int type);
void printArray(int arr[], int n);
int* importFromFile(int* n);
const char* getDistName(int type);
void saveResultsToFile(SortPerformance results[], int count, int type);
SortPerformance testAlgorithm(void (*sortFunc)(int*, int), int rawData[], int n, char* name);

#endif // HEAD_H