#include"head.h"

long g_compareCount = 0;
long g_moveCount = 0;
int g_visualize = 0;
int g_delay = 1000;
LARGE_INTEGER g_cpuFreq;

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
        printf("             可视化排序算法分析系统         \n");
        printf("============================================\n");
        printf("1. 设置规模/生成数据 (当前: %d, 类型: %s)\n", n, getDistName(type));
        printf("2. 导入测试用例\n");
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

            printArray(rawData, n);
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

                printArray(rawData, n);
            }
            break;
        }
        case 3:
            g_visualize = !g_visualize;
            break;
        case 4: {
            printf("算法: 1.冒泡 2.插入 3.选择 4.快速 5.希尔 6.堆 7.归并: ");
            int algo; scanf_s("%d", &algo);
            SortPerformance sp;
            memset(&sp, 0, sizeof(sp));

            if (algo == 1) sp = testAlgorithm(BubbleSort, rawData, n, "Bubble");
            else if (algo == 2) sp = testAlgorithm(InsertSort, rawData, n, "Insert");
            else if (algo == 3) sp = testAlgorithm(SelectSort, rawData, n, "Select");
            else if (algo == 4) sp = testAlgorithm(QuickSortIterative, rawData, n, "Quick");
            else if (algo == 5) sp = testAlgorithm(ShellSort, rawData, n, "Shell");
            else if (algo == 6) sp = testAlgorithm(HeapSort, rawData, n, "Heap");
            else if (algo == 7) sp = testAlgorithm(MergeSort, rawData, n, "Merge");
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
            results[3] = testAlgorithm(QuickSortIterative, rawData, n, "Quick");
            results[4] = testAlgorithm(ShellSort, rawData, n, "Shell");
            results[5] = testAlgorithm(HeapSort, rawData, n, "Heap");
            results[6] = testAlgorithm(MergeSort, rawData, n, "Merge");

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