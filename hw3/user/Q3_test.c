#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define TYPE_CPU_INTENSIVE 0
#define TYPE_IO_INTENSIVE 1
#define TYPE_MIXED 2

void busy_wait(int ticks) {
    int i = 0;
    while (i < ticks) {
        i++;
    }
}

void cpu_worker(int id, int duration) {
    int pid = getpid();
    int work_units = duration / 100;
    
    printf("[START] PID %d (CPU Worker %d) started - will do %d work units\n", 
                  pid, id, work_units);
    
    for (int i = 0; i < work_units; i++) {
        busy_wait(1000 * work_units); 
        if (i % 100 == 0) {
            // Optional: print progress
            // printf("[OBSERVE] PID %d completed %d/%d work units\n", 
            //              pid, i, work_units);
        }
    }
    
    printf("[FINISH] PID %d (CPU Worker %d) completed all %d work units\n", 
                  pid, id, work_units);
    exit(0);
}

void io_worker(int id, int duration) {
    int pid = getpid();
    int io_operations = duration / 200;
    
    printf("[START] PID %d (IO Worker %d) started - will do %d IO operations\n", 
                  pid, id, io_operations);
    
    for (int i = 0; i < io_operations; i++) {
        busy_wait(50);   
        // printf("[OBSERVE] PID %d completed CPU operation %d/%d\n", 
        //              pid, i + 1, io_operations);
        sleep(1);        
        // printf("[OBSERVE] PID %d completed IO operation %d/%d\n", 
        //              pid, i + 1, io_operations);
    }
    
    printf("[FINISH] PID %d (IO Worker %d) completed all %d IO operations\n", 
                  pid, id, io_operations);
    exit(0);
}

void mixed_worker(int id, int duration) {
    int pid = getpid();
    int cycles = duration / 300;
    
    printf("[START] PID %d (Mixed Worker %d) started - will do %d cycles\n", 
                  pid, id, cycles);
    
    for (int i = 0; i < cycles; i++) {
        busy_wait(150);
        sleep(8);
        busy_wait(50);
    }
    printf("[FINISH] PID %d (Mixed Worker %d) completed all %d cycles\n", 
                  pid, id, cycles);
    exit(0);
}

int main(int argc, char *argv[]) {
    
    printf("\n");
    printf("===============================================\n");
    printf("=        MLFQ Scheduler Test Program         =\n");
    printf("=        Testing 3-Level Feedback Queue      =\n");
    printf("===============================================\n");
    printf("===============================================\n\n");
    
    printf("[TEST] Starting basic MLFQ test with mixed workloads\n");
    printf("[TEST] Expected behavior:\n");
    printf("       - CPU-intensive processes should demote Q0->Q1->Q2\n");
    printf("       - I/O-bound processes should stay in high queues\n");
    printf("       - All processes boost to Q0 every 100 ticks\n\n");
    
    // Create test processes
    if (fork() == 0) cpu_worker(1, 2000000000);    // CPU-intensive
    if (fork() == 0) io_worker(1, 20000);          // I/O-intensive
    if (fork() == 0) cpu_worker(2, 2500000000);    // CPU-intensive
    if (fork() == 0) cpu_worker(3, 1800000000);    // CPU-intensive
    if (fork() == 0) io_worker(2, 22000);          // I/O-intensive
    
    int children = 5;
    
    printf("[TEST] Waiting for %d child processes to complete...\n\n", children);
    
    int completed = 0;
    while (completed < children) {
        int status;
        int pid = wait(&status);
        if (pid > 0) {
            printf("[TEST] Child PID %d exited with status %d\n", pid, status);
            completed++;
        }
    }
    
    printf("\n");
    printf("===============================================\n");
    printf("=           Test Completed Successfully      =\n");
    printf("=           Processes completed: %d          =\n", completed);
    printf("===============================================\n\n");
    
    exit(0);
}

