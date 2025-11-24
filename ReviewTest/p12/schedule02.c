#include <stdio.h>
#include <limits.h>

//非抢占式SJF调度算法实现，短作业优先调度算法
// 定义进程结构
typedef struct {
    int id;
    int arrival_time;
    int burst_time;
    int completion_time;
    int turnaround_time;
    float weighted_turnaround_time;
    int waiting_time;
    int is_completed; // 标记进程是否已完成
} Process;

void sjf_scheduling(Process processes[], int n) {
    int current_time = 0;
    int completed_processes = 0;

    while (completed_processes < n) {
        // 1. 找到当前时间点已到达且未完成的进程中，服务时间最短的那个
        int shortest_job_index = -1;
        int shortest_burst_time = INT_MAX;

        for (int i = 0; i < n; i++) {
            if (processes[i].arrival_time <= current_time && !processes[i].is_completed) {
                if (processes[i].burst_time < shortest_burst_time) {
                    shortest_burst_time = processes[i].burst_time;
                    shortest_job_index = i;
                }
            }
        }

        // 如果没有找到（所有已到达的进程都已完成，但还有进程未到达）
        if (shortest_job_index == -1) {
            current_time++;
            continue;
        }

        // 2. 执行选定的短作业
        Process* p = &processes[shortest_job_index];
        p->completion_time = current_time + p->burst_time;
        current_time = p->completion_time;
        p->turnaround_time = p->completion_time - p->arrival_time;
        p->waiting_time = p->turnaround_time - p->burst_time;
        p->weighted_turnaround_time = (float)p->turnaround_time / p->burst_time;
        p->is_completed = 1;

        completed_processes++;
    }
}

// (print_results 函数与上一个例子相同，此处省略以避免重复)
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
    Process processes[] = {
        {1, 0, 5, 0, 0, 0.0f, 0, 0},
        {2, 1, 3, 0, 0, 0.0f, 0, 0},
        {3, 2, 1, 0, 0, 0.0f, 0, 0},
        {4, 3, 2, 0, 0, 0.0f, 0, 0},
        {5, 4, 4, 0, 0, 0.0f, 0, 0}
    };

    int n = sizeof(processes) / sizeof(processes[0]);

    sjf_scheduling(processes, n);
    print_results(processes, n, "SJF (Non-Preemptive)");

    return 0;
}