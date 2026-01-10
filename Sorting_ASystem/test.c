#include <stdlib.h>

// 宏定义
#ifdef _WIN32
    #define EXPORT __declspec(dllexport)
#else
    #define EXPORT
#endif

// 一个加法测试函数
EXPORT int add_numbers(int a, int b) {
    return a + b;
}