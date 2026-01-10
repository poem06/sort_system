/*
 * 文件名: main.c
 * 描述: 主程序入口，处理用户交互
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "sort_system.h"

 // 打印表头
void print_report(SortPerformance p) {
    printf("| %-12s | %-12lld | %-12lld | %-12.6f |\n",
        p.algo_name, p.comparisons, p.moves, p.time_used);
}

int main() {
    srand((unsigned int)time(NULL));
    int* data = NULL;
    int* temp_data = NULL;
    int n = 0;
    int choice;
    bool has_data = false;

    printf("========================================\n");
    printf("   可视化排序算法分析系统 (第四组)   \n");
    printf("========================================\n");

    while (1) {
        printf("\n--- 功能菜单 ---\n");
        printf("1. 生成/输入数据\n");
        printf("2. 运行单项算法可视化\n");
        printf("3. 批量性能对比分析报告\n");
        printf("0. 退出\n");
        printf("请选择: ");

        // 使用 scanf_s 读取整数
        if (scanf_s("%d", &choice) != 1) {
            // 处理非法输入，清除缓冲区
            while (getchar() != '\n');
            continue;
        }

        if (choice == 0) break;

        if (choice == 1) {
            printf("请输入数据规模 N: ");
            scanf_s("%d", &n);
            if (n <= 0) { printf("规模必须大于0\n"); continue; }

            if (data) free(data);
            if (temp_data) free(temp_data);
            data = (int*)malloc(n * sizeof(int));
            temp_data = (int*)malloc(n * sizeof(int));

            printf("1. 随机生成\n2. 手动输入\n选择: ");
            int sub_choice;
            scanf_s("%d", &sub_choice);

            if (sub_choice == 1) {
                generate_data(data, n, 10000);
            }
            else {
                printf("请输入 %d 个整数:\n", n);
                for (int i = 0; i < n; i++) {
                    scanf_s("%d", &data[i]);
                }
            }
            has_data = true;
            printf("当前数据: ");
            if (n <= 20) print_array(data, n);
            else printf("(数据量过大，隐藏显示)\n");
        }
        else if (choice == 2) {
            if (!has_data) { printf("请先生成数据！\n"); continue; }

            copy_array(data, temp_data, n);

            printf("\n选择算法进行可视化演示 (建议 N < 20):\n");
            printf("1. 插入排序\n2. 冒泡排序\n3. 选择排序\n4. 希尔排序\n5. 快速排序\n选择: ");
            int algo_choice;
            scanf_s("%d", &algo_choice);

            printf("\n--- 开始可视化排序 ---\n");
            printf("初始状态: ");
            if (n <= 50) print_array(temp_data, n); else printf("...\n");

            switch (algo_choice) {
            case 1: insertion_sort(temp_data, n, true); break;
            case 2: bubble_sort(temp_data, n, true); break;
            case 3: selection_sort(temp_data, n, true); break;
            case 4: shell_sort(temp_data, n, true); break;
            case 5: quick_sort_recursive(temp_data, 0, n - 1, n, true); break;
            }
            printf("排序完成!\n");
        }
        else if (choice == 3) {
            if (!has_data) { printf("请先生成数据！\n"); continue; }

            printf("\n正在进行批量测试 (无可视化)...\n");
            printf("-------------------------------------------------------------\n");
            printf("| 算法名称     | 比较次数     | 移动次数     | 耗时(毫秒)   |\n");
            printf("-------------------------------------------------------------\n");

            // 依次测试所有算法
            // 插入
            copy_array(data, temp_data, n);
            SortPerformance p1 = run_algorithm(insertion_sort, NULL, temp_data, n, "插入排序", false, false);
            print_report(p1);

            // 冒泡
            copy_array(data, temp_data, n);
            SortPerformance p2 = run_algorithm(bubble_sort, NULL, temp_data, n, "冒泡排序", false, false);
            print_report(p2);

            // 选择
            copy_array(data, temp_data, n);
            SortPerformance p3 = run_algorithm(selection_sort, NULL, temp_data, n, "选择排序", false, false);
            print_report(p3);

            // 希尔
            copy_array(data, temp_data, n);
            SortPerformance p4 = run_algorithm(shell_sort, NULL, temp_data, n, "希尔排序", false, false);
            print_report(p4);

            // 快排
            copy_array(data, temp_data, n);
            SortPerformance p5 = run_algorithm(NULL, quick_sort_recursive, temp_data, n, "快速排序", true, false);
            print_report(p5);

            printf("-------------------------------------------------------------\n");
        }
    }

    if (data) free(data);
    if (temp_data) free(temp_data);
    return 0;
}