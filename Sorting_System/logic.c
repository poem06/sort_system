#include"head.h"


// 2. 栈操作函数

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
    if (s->data) {
        free(s->data);
        s->data = NULL;
    }
}

// 3. 基础辅助函数

// 初始化高精度计时器
void initTimer() {
    QueryPerformanceFrequency(&g_cpuFreq);
}

// 获取当前微秒级时间戳
long long getTick() {
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);
    return t.QuadPart;
}

// 计算时间差(秒)
double calcDuration(long long start, long long end) {
    return (double)(end - start) / g_cpuFreq.QuadPart;
}

// 可视化展示 
void visualize(int arr[], int n) {
    if (!g_visualize) return;
    system("cls");
    printf("--- 排序可视化中 ---\n");
    for (int i = 0; i < n; i++) {
        printf("%3d | ", arr[i]);
        // 简单柱状图
        if (n <= 30) {
            for (int k = 0; k < arr[i]; k++) printf("*");
        }
        printf("\n");
    }
    Sleep(g_delay);
}

// 交换元素
void swap(int* a, int* b) {
    int temp = *a; *a = *b; *b = temp;
    g_moveCount += 3; // 交换计为3次移动 
}

// 数组拷贝
void copyArray(int src[], int dest[], int n) {
    for (int i = 0; i < n; i++) dest[i] = src[i];
}

// 辅助函数：取最小值
int min_val(int x, int y) {
    return x < y ? x : y;
}

// 4. 排序算法实现

// 4.1 冒泡排序
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

// 4.2 直接插入排序
void InsertSort(int arr[], int n) {
    int i, j, temp;
    for (i = 1; i < n; i++) {
        temp = arr[i]; g_moveCount++;
        j = i - 1; g_compareCount++;
        while (j >= 0 && arr[j] > temp) {
            g_compareCount++;
            arr[j + 1] = arr[j]; g_moveCount++;
            j--;
            visualize(arr, n);
        }
        arr[j + 1] = temp; g_moveCount++;
        visualize(arr, n);
    }
}

// 4.3 直接选择排序 
void SelectSort(int arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        int minIdx = i;
        for (int j = i + 1; j < n; j++) {
            g_compareCount++;
            if (arr[j] < arr[minIdx]) minIdx = j;
        }
        if (minIdx != i) {
            swap(&arr[i], &arr[minIdx]);
            visualize(arr, n);
        }
    }
}

// 4.4 希尔排序
void ShellSort(int arr[], int n) {
    int gap, i, j, temp;
    for (gap = n / 2; gap > 0; gap /= 2) {
        for (i = gap; i < n; i++) {
            temp = arr[i]; g_moveCount++;
            j = i; g_compareCount++;
            while (j >= gap && arr[j - gap] > temp) {
                g_compareCount++;
                arr[j] = arr[j - gap]; g_moveCount++;
                j -= gap;
                visualize(arr, n);
            }
            arr[j] = temp; g_moveCount++;
        }
    }
}

// 4.5 堆排序
void HeapAdjustIterative(int arr[], int n, int i) {
    int temp = arr[i]; g_moveCount++;
    int k = 2 * i + 1; // 左孩子

    while (k < n) {
        g_compareCount++;
        // 找最大的孩子
        if (k + 1 < n && arr[k] < arr[k + 1]) {
            k++;
        }

        g_compareCount++;
        if (arr[k] > temp) {
            arr[i] = arr[k]; g_moveCount++;
            i = k; // 继续向下寻找位置
            k = 2 * i + 1;
        }
        else {
            break;
        }
    }
    arr[i] = temp; g_moveCount++;
}

void HeapSort(int arr[], int n) {
    // 1. 建堆
    for (int i = n / 2 - 1; i >= 0; i--) {
        HeapAdjustIterative(arr, n, i);
        visualize(arr, n);
    }
    // 2. 排序
    for (int i = n - 1; i > 0; i--) {
        swap(&arr[0], &arr[i]);
        visualize(arr, n);
        HeapAdjustIterative(arr, i, 0);
        visualize(arr, n);
    }
}

// 4.6 归并排序
void Merge(int arr[], int l, int m, int r, int temp[]) {
    int i = l, j = m + 1, k = l;
    while (i <= m && j <= r) {
        g_compareCount++;
        if (arr[i] <= arr[j]) { temp[k++] = arr[i++]; g_moveCount++; }
        else { temp[k++] = arr[j++]; g_moveCount++; }
    }
    while (i <= m) { temp[k++] = arr[i++]; g_moveCount++; }
    while (j <= r) { temp[k++] = arr[j++]; g_moveCount++; }

    // 拷贝回原数组
    for (i = l; i <= r; i++) { arr[i] = temp[i]; g_moveCount++; }
}

void MergeSort(int arr[], int n) {
    int* temp = (int*)malloc(n * sizeof(int));
    if (!temp) return;

    // curr_size: 当前合并子序列的大小 1 -> 2 -> 4 -> 8...
    for (int curr_size = 1; curr_size <= n - 1; curr_size = 2 * curr_size) {
        // left_start: 当前合并组的起始位置
        for (int left_start = 0; left_start < n - 1; left_start += 2 * curr_size) {
            int mid = min_val(left_start + curr_size - 1, n - 1);
            int right_end = min_val(left_start + 2 * curr_size - 1, n - 1);

            Merge(arr, left_start, mid, right_end, temp);
            visualize(arr, n);
        }
    }
    free(temp);
}

// 4.7 快速排序
int Partition(int arr[], int low, int high, int n_visual) {
    int pivot = arr[low]; g_moveCount++;
    while (low < high) {
        while (low < high && arr[high] >= pivot) { g_compareCount++; high--; }
        g_compareCount++; arr[low] = arr[high]; g_moveCount++;
        while (low < high && arr[low] <= pivot) { g_compareCount++; low++; }
        g_compareCount++; arr[high] = arr[low]; g_moveCount++;
        visualize(arr, n_visual);
    }
    arr[low] = pivot; g_moveCount++;
    return low;
}

void QuickSortIterative(int arr[], int n) {
    if (n <= 1) return;

    SeqStack stack;
    initStack(&stack, n); // 初始化栈
    push(&stack, 0, n - 1); // 压入初始区间

    int low, high, pivot;

    while (!isStackEmpty(&stack)) {
        pop(&stack, &low, &high);

        pivot = Partition(arr, low, high, n);

        // 如果右边有元素，压栈
        if (pivot + 1 < high) {
            push(&stack, pivot + 1, high);
        }
        // 如果左边有元素，压栈
        if (low < pivot - 1) {
            push(&stack, low, pivot - 1);
        }
    }
    freeStack(&stack);
}

// 5. 数据管理与IO模块 
void generateData(int arr[], int n, int type) {
    srand((unsigned)time(NULL));
    for (int i = 0; i < n; i++) arr[i] = rand() % 100 + 1;
    if (type == 1) { // Sorted
        for (int i = 0; i < n - 1; i++) for (int j = 0; j < n - 1 - i; j++)
            if (arr[j] > arr[j + 1]) { int t = arr[j]; arr[j] = arr[j + 1]; arr[j + 1] = t; }
    }
    else if (type == 2) { // Reverse
        for (int i = 0; i < n - 1; i++) for (int j = 0; j < n - 1 - i; j++)
            if (arr[j] < arr[j + 1]) { int t = arr[j]; arr[j] = arr[j + 1]; arr[j + 1] = t; }
    }
}

void printArray(int arr[], int n) {
    printf("\n--- 当前数据预览 ---\n");
    int limit = (n > 100) ? 100 : n; // 如果数据超过100个，只显示前100个，防止刷屏
    for (int i = 0; i < limit; i++) {
        printf("%-4d ", arr[i]); // 打印数值，占4位左对齐
        if ((i + 1) % 10 == 0) printf("\n"); // 每10个换行
    }
    if (n > 100) {
        printf("\n... (剩余 %d 个数据已隐藏)\n", n - 100);
    }
    else {
        printf("\n");
    }
    printf("--------------------\n");
}

// 修改后的导入函数
int* importFromFile(int* n) {
    char filename[100];
    printf("请输入文件名 (例如 sort_data.txt): ");
    scanf_s("%s", filename, (unsigned)sizeof(filename));

    FILE* fp;
    if (fopen_s(&fp, filename, "r") != 0) {
        printf("错误：无法打开文件 %s\n", filename);
        *n = 0;
        return NULL;
    }

    // --- 第一步：扫描文件计算数据个数 ---
    int count = 0;
    int tempVal;
    char ch;

    // 循环读取：尝试读一个整数，然后尝试读一个字符(逗号)
    while (fscanf_s(fp, "%d", &tempVal) == 1) {
        count++;
        // 读取下一个字符（可能是逗号，也可能是EOF）
        ch = fgetc(fp);
        // 如果不是数字也不是负号，继续读直到找到下一个数字或文件结束
        while (ch != EOF && (ch < '0' || ch > '9') && ch != '-') {
            ch = fgetc(fp);
        }
        // 回退一个字符，以便下一次 fscanf 能读到数字
        if (ch != EOF) {
            ungetc(ch, fp);
        }
    }

    *n = count;
    printf("检测到文件中共有 %d 个数据。\n", count);

    if (count == 0) {
        fclose(fp);
        return NULL;
    }

    // --- 第二步：分配内存并重新读取 ---
    int* arr = (int*)malloc(count * sizeof(int));

    // 文件指针回到开头
    rewind(fp);

    for (int i = 0; i < count; i++) {
        fscanf_s(fp, "%d", &arr[i]);
        // 跳过后面的逗号或空格，为读取下一个数字做准备
        ch = fgetc(fp);
        while (ch != EOF && (ch < '0' || ch > '9') && ch != '-') {
            ch = fgetc(fp);
        }
        if (ch != EOF) {
            ungetc(ch, fp);
        }
    }

    fclose(fp);
    printf("成功导入数据。\n");
    return arr;
}

// 辅助：获取分布名称
const char* getDistName(int type) {
    if (type == 1) return "Sorted";
    if (type == 2) return "Reverse";
    return "Random";
}

// 导出结果
void saveResultsToFile(SortPerformance results[], int count, int type) {
    FILE* fp;
    char filename[] = "sort_report.csv";

    // 获取当前系统时间
    time_t rawtime;
    struct tm timeinfo;
    char timeStr[30];
    time(&rawtime);
    localtime_s(&timeinfo, &rawtime);
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);

    // 追加模式打开
    if (fopen_s(&fp, filename, "a") != 0) {
        printf("错误：无法写入文件\n");
        return;
    }

    // 若文件为空，写入表头
    fseek(fp, 0, SEEK_END);
    if (ftell(fp) == 0) {
        fprintf(fp, "BatchTime,DataType,Algorithm,DataSize,Comparisons,Moves,Time(s)\n");
    }

    // 写入数据
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%s,%s,%s,%d,%ld,%ld,%.8f\n",
            timeStr,
            getDistName(type),
            results[i].algorithm,
            results[i].dataSize,
            results[i].compareCount,
            results[i].moveCount,
            results[i].timeCost);
    }

    fclose(fp);
    printf("结果已导出至 %s \n", filename);
}

// 算法测试驱动
SortPerformance testAlgorithm(void (*sortFunc)(int*, int), int rawData[], int n, char* name) {
    int* arr = (int*)malloc(n * sizeof(int));
    copyArray(rawData, arr, n);

    g_compareCount = 0;
    g_moveCount = 0;

    long long start = getTick();
    sortFunc(arr, n);
    long long end = getTick();

    SortPerformance sp;
    strcpy_s(sp.algorithm, sizeof(sp.algorithm), name);
    sp.dataSize = n;
    sp.compareCount = g_compareCount;
    sp.moveCount = g_moveCount;
    sp.timeCost = calcDuration(start, end);

    free(arr);
    return sp;
}