#include <stdio.h>
#include <stdlib.h> // 新增：用于 calloc 和 free

#define TIME_QUANTUM 2

// 定义进程结构（需与初始化列表的元素数量严格匹配）
typedef struct Process {
    int id;
    int arrival_time;
    int burst_time;
    int remaining_time; // 剩余服务时间
    int completion_time;
    int turnaround_time;
    int waiting_time;
    float weighted_turnaround_time;
} Process;

// 定义循环队列用于就绪队列
#define MAX_QUEUE_SIZE 100
int queue[MAX_QUEUE_SIZE];
int front = -1, rear = -1;

void enqueue(int pid) {
    if (rear == MAX_QUEUE_SIZE - 1) return;
    if (front == -1) front = 0;
    rear++;
    queue[rear] = pid;
}

int dequeue() {
    if (front == -1 || front > rear) return -1;
    int pid = queue[front];
    front++;
    if (front > rear) front = rear = -1; // 重置队列
    return pid;
}

int is_queue_empty() {
    return front == -1;
}

void rr_scheduling(Process processes[], int n) {
    int current_time = 0;
    int completed_processes = 0;
    int* is_in_queue = (int*)calloc(n, sizeof(int)); // 标记进程是否在就绪队列中

    // 初始化剩余时间
    for (int i = 0; i < n; i++) {
        processes[i].remaining_time = processes[i].burst_time;
    }

    // 将第一个到达的进程加入队列
    for (int i = 0; i < n; i++) {
        if (processes[i].arrival_time <= current_time) {
            enqueue(i);
            is_in_queue[i] = 1;
            break; // 假设按到达时间排序
        }
    }

    while (completed_processes < n) {
        if (is_queue_empty()) {
            current_time++;
            // 检查新进程到达
            for (int i = 0; i < n; i++) {
                if (processes[i].arrival_time == current_time && !is_in_queue[i] && processes[i].remaining_time > 0) {
                    enqueue(i);
                    is_in_queue[i] = 1;
                }
            }
            continue;
        }

        int current_pid_index = dequeue();
        is_in_queue[current_pid_index] = 0;
        Process* p = &processes[current_pid_index];

        // 执行时间片
        int time_to_execute = (p->remaining_time < TIME_QUANTUM) ? p->remaining_time : TIME_QUANTUM;
        
        current_time += time_to_execute;
        p->remaining_time -= time_to_execute;

        // 检查并加入新到达的进程
        for (int i = 0; i < n; i++) {
            if (processes[i].arrival_time <= current_time && !is_in_queue[i] && processes[i].remaining_time > 0 && i != current_pid_index) {
                enqueue(i);
                is_in_queue[i] = 1;
            }
        }

        // 进程执行完毕
        if (p->remaining_time == 0) {
            p->completion_time = current_time;
            completed_processes++;
        } else { // 放回就绪队列尾部
            enqueue(current_pid_index);
            is_in_queue[current_pid_index] = 1;
        }
    }
    
    free(is_in_queue);

    // 计算性能指标
    for (int i = 0; i < n; i++) {
        processes[i].turnaround_time = processes[i].completion_time - processes[i].arrival_time;
        processes[i].waiting_time = processes[i].turnaround_time - processes[i].burst_time;
        processes[i].weighted_turnaround_time = (float)processes[i].turnaround_time / processes[i].burst_time;
    }
}

// 打印调度结果
void print_results(Process processes[], int n, const char* algorithm_name) {
    printf("\n=== %s Scheduling Results ===\n", algorithm_name);
    printf("PID\tAT\tBT\t剩余BT\tCT\tTAT\tWT\tWTAT\n");
    printf("-----------------------------------------------------\n");

    float avg_tat = 0, avg_wt = 0, avg_wtat = 0;

    for (int i = 0; i < n; i++) {
        printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%.2f\n",
               processes[i].id,
               processes[i].arrival_time,
               processes[i].burst_time,
               processes[i].remaining_time, // 打印剩余服务时间
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
    // 初始化进程（元素数量与结构体定义严格匹配）
    Process processes[] = {
        {1, 0, 5, 5, 0, 0, 0, 0.0f},
        {2, 1, 3, 3, 0, 0, 0, 0.0f},
        {3, 2, 1, 1, 0, 0, 0, 0.0f},
        {4, 3, 2, 2, 0, 0, 0, 0.0f},
        {5, 4, 4, 4, 0, 0, 0, 0.0f}
    };

    int n = sizeof(processes) / sizeof(processes[0]);

    rr_scheduling(processes, n);
    print_results(processes, n, "Round-Robin (RR)");

    return 0;
}