#include <stdio.h>
#include <limits.h>

// 抢占式SRTF调度算法实现，最短剩余时间优先调度算法
typedef struct {
    int id;
    int arrival_time;
    int burst_time;         // 剩余服务时间
    int original_burst_time;// 原始服务时间（初始值，不修改）
    int completion_time;
    int turnaround_time;
    float weighted_turnaround_time;
    int waiting_time;
    int is_completed;       // 标记进程是否已完成
} Process;

void srtf_scheduling(Process processes[], int n) {
    int current_time = 0;
    int completed_processes = 0;

    while (completed_processes < n) {
        // 1. 找到当前时间点已到达且未完成的进程中，剩余服务时间最短的那个
        int shortest_remaining_index = -1;
        int shortest_remaining_time = INT_MAX;

        for (int i = 0; i < n; i++) {
            if (processes[i].arrival_time <= current_time && !processes[i].is_completed) {
                if (processes[i].burst_time < shortest_remaining_time) {
                    shortest_remaining_time = processes[i].burst_time;
                    shortest_remaining_index = i;
                }
            }
        }

        if (shortest_remaining_index == -1) {
            current_time++;
            continue;
        }

        // 2. 执行选定的进程一个时间单位
        Process* p = &processes[shortest_remaining_index];
        p->burst_time--; // 剩余服务时间减一

        current_time++;

        // 3. 如果该进程执行完毕
        if (p->burst_time == 0) {
            p->completion_time = current_time;
            p->turnaround_time = p->completion_time - p->arrival_time;
            // 用原始服务时间计算等待时间
            p->waiting_time = p->turnaround_time - p->original_burst_time;
            p->weighted_turnaround_time = (float)p->turnaround_time / p->original_burst_time;
            p->is_completed = 1;

            completed_processes++;
        }
    }
}

// 打印调度结果
void print_results(Process processes[], int n, const char* algorithm_name) {
    printf("\n=== %s Scheduling Results ===\n", algorithm_name);
    printf("PID\tAT\t原始BT\t剩余BT\tCT\tTAT\tWT\tWTAT\n");
    printf("-----------------------------------------------------\n");

    float avg_tat = 0, avg_wt = 0, avg_wtat = 0;

    for (int i = 0; i < n; i++) {
        printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%.2f\n",
               processes[i].id,
               processes[i].arrival_time,
               processes[i].original_burst_time, // 打印原始服务时间
               processes[i].burst_time,         // 打印剩余服务时间（SRTF修改后的值）
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

    printf("-----------------------------------------------------\n");
    printf("Average TAT: %.2f\n", avg_tat);
    printf("Average WT:  %.2f\n", avg_wt);
    printf("Average WTAT: %.2f\n", avg_wtat);
}

int main() {
    // 初始化进程，同时设置原始服务时间
    Process processes[] = {
        {1, 0, 5, 5, 0, 0, 0.0f, 0, 0},
        {2, 1, 3, 3, 0, 0, 0.0f, 0, 0},
        {3, 2, 1, 1, 0, 0, 0.0f, 0, 0},
        {4, 3, 2, 2, 0, 0, 0.0f, 0, 0},
        {5, 4, 4, 4, 0, 0, 0.0f, 0, 0}
    };

    int n = sizeof(processes) / sizeof(processes[0]);

    srtf_scheduling(processes, n);
    print_results(processes, n, "SRTF (Preemptive)");

    return 0;
}