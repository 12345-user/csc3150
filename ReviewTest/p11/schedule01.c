#include <stdio.h>


//FCFS调度算法实现
// 定义进程结构
typedef struct {
    int id;
    int arrival_time;
    int burst_time;
    int completion_time;
    int turnaround_time;
    float weighted_turnaround_time;
    int waiting_time;
} Process;

// FCFS 调度函数
void fcfs_scheduling(Process processes[], int n) {
    int current_time = 0;

    for (int i = 0; i < n; i++) {
        // 如果当前时间小于进程到达时间，则 CPU 空闲等待
        if (current_time < processes[i].arrival_time) {
            current_time = processes[i].arrival_time;
        }

        // 进程开始执行
        processes[i].completion_time = current_time + processes[i].burst_time;
        current_time = processes[i].completion_time;

        // 计算性能指标
        processes[i].turnaround_time = processes[i].completion_time - processes[i].arrival_time;
        processes[i].waiting_time = processes[i].turnaround_time - processes[i].burst_time;
        processes[i].weighted_turnaround_time = (float)processes[i].turnaround_time / processes[i].burst_time;
    }
}

// 打印调度结果
void print_results(Process processes[], int n, const char* algorithm_name) {
    printf("\n=== %s Scheduling Results ===\n", algorithm_name);
    printf("PID\tAT\tBT\tCT\tTAT\tWT\tWTAT\n");
    printf("---------------------------------------------\n");

    float avg_tat = 0, avg_wt = 0, avg_wtat = 0;

    for (int i = 0; i < n; i++) {
        printf("%d\t%d\t%d\t%d\t%d\t%d\t%.2f\n",
               processes[i].id,
               processes[i].arrival_time,
               processes[i].burst_time,
               processes[i].completion_time,
               processes[i].turnaround_time,
               processes[i].waiting_time,
               processes[i].weighted_turnaround_time);

        avg_tat += processes[i].turnaround_time;
        avg_wt += processes[i].waiting_time;
        avg_wtat += processes[i].weighted_turnaround_time;
    }

    avg_tat /= n;
    avg_wt /= n;
    avg_wtat /= n;

    printf("---------------------------------------------\n");
    printf("Average TAT: %.2f\n", avg_tat);
    printf("Average WT:  %.2f\n", avg_wt);
    printf("Average WTAT: %.2f\n", avg_wtat);
}

int main() {
    // 测试案例数据
    Process processes[] = {
        {1, 0, 5, 0, 0, 0.0f, 0},
        {2, 1, 3, 0, 0, 0.0f, 0},
        {3, 2, 1, 0, 0, 0.0f, 0},
        {4, 3, 2, 0, 0, 0.0f, 0},
        {5, 4, 4, 0, 0, 0.0f, 0}
    };

    int n = sizeof(processes) / sizeof(processes[0]);

    // 注意：FCFS 要求进程按到达时间排序
    // 此处我们的输入数据已经是按到达时间排序的

    fcfs_scheduling(processes, n);
    print_results(processes, n, "FCFS");

    return 0;
}