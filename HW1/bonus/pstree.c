#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_PROCESSES 1000
#define MAX_NAME_LEN 256
#define MAX_LINE_LEN 1024

typedef struct {
    int pid;
    int ppid;
    char name[MAX_NAME_LEN];
    int children[MAX_PROCESSES];
    int child_count;
} Process;

Process processes[MAX_PROCESSES];
int process_count = 0;
int show_pids = 0;
int show_threads = 0;
int numeric_sort = 0;
int version_info = 0;
int help_info = 0;

// 从/proc/pid/stat读取进程信息
int read_process_info(int pid, Process *proc) {
    char path[256];
    char line[MAX_LINE_LEN];
    FILE *file;
    
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    file = fopen(path, "r");
    if (!file) return 0;
    
    if (fgets(line, sizeof(line), file)) {
        char *token = strtok(line, " ");
        int field = 1;
        
        while (token) {
            if (field == 1) {
                proc->pid = atoi(token);
            } else if (field == 2) {
                // 进程名，去掉括号
                strncpy(proc->name, token + 1, MAX_NAME_LEN - 1);
                proc->name[strlen(proc->name) - 1] = '\0';
            } else if (field == 4) {
                proc->ppid = atoi(token);
                break;
            }
            token = strtok(NULL, " ");
            field++;
        }
    }
    
    fclose(file);
    proc->child_count = 0;
    return 1;
}

// 扫描所有进程
void scan_processes() {
    DIR *proc_dir;
    struct dirent *entry;
    
    proc_dir = opendir("/proc");
    if (!proc_dir) {
        perror("opendir /proc");
        exit(1);
    }
    
    process_count = 0;
    while ((entry = readdir(proc_dir)) != NULL) {
        int pid = atoi(entry->d_name);
        if (pid > 0 && process_count < MAX_PROCESSES) {
            if (read_process_info(pid, &processes[process_count])) {
                process_count++;
            }
        }
    }
    
    closedir(proc_dir);
}

// 建立父子关系
void build_tree() {
    for (int i = 0; i < process_count; i++) {
        for (int j = 0; j < process_count; j++) {
            if (processes[j].ppid == processes[i].pid) {
                if (processes[i].child_count < MAX_PROCESSES) {
                    processes[i].children[processes[i].child_count++] = j;
                }
            }
        }
    }
}

// 查找进程索引
int find_process(int pid) {
    for (int i = 0; i < process_count; i++) {
        if (processes[i].pid == pid) {
            return i;
        }
    }
    return -1;
}

// 打印进程树
void print_tree(int proc_index, char *prefix, int is_last) {
    Process *proc = &processes[proc_index];
    
    printf("%s", prefix);
    
    if (strlen(prefix) > 0) {
        printf("%s", is_last ? "└─" : "├─");
    }
    
    if (show_pids) {
        printf("%s(%d)", proc->name, proc->pid);
    } else {
        printf("%s", proc->name);
    }
    
    if (proc->child_count > 0) {
        printf("─┬─");
        
        // 打印第一个子进程
        if (proc->child_count > 0) {
            char new_prefix[1024];
            snprintf(new_prefix, sizeof(new_prefix), "%s%s", prefix, 
                    strlen(prefix) > 0 ? (is_last ? "  " : "│ ") : "");
            
            for (int i = 0; i < proc->child_count; i++) {
                if (i == 0) {
                    printf("\n");
                }
                print_tree(proc->children[i], new_prefix, i == proc->child_count - 1);
                if (i < proc->child_count - 1) {
                    printf("\n");
                }
            }
        }
    }
    
    if (proc->child_count == 0) {
        printf("\n");
    }
}

// 显示帮助信息
void show_help() {
    printf("Usage: pstree [options]\n");
    printf("Options:\n");
    printf("  -p, --show-pids    Show process IDs\n");
    printf("  -n, --numeric-sort Sort by PID\n");
    printf("  -T, --hide-threads Hide threads\n");
    printf("  -V, --version      Show version\n");
    printf("  -h, --help         Show this help\n");
}

// 显示版本信息
void show_version() {
    printf("pstree (custom implementation) 1.0\n");
    printf("Copyright (C) 2024 SC3150 Assignment\n");
}

int main(int argc, char *argv[]) {
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--show-pids") == 0) {
            show_pids = 1;
        } else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--numeric-sort") == 0) {
            numeric_sort = 1;
        } else if (strcmp(argv[i], "-T") == 0 || strcmp(argv[i], "--hide-threads") == 0) {
            show_threads = 0;
        } else if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--version") == 0) {
            show_version();
            return 0;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            show_help();
            return 0;
        }
    }
    
    // 扫描进程
    scan_processes();
    
    // 建立进程树
    build_tree();
    
    // 查找init进程 (PID 1)
    int init_index = find_process(1);
    if (init_index == -1) {
        // 如果找不到init，找第一个没有父进程的进程
        for (int i = 0; i < process_count; i++) {
            if (find_process(processes[i].ppid) == -1) {
                init_index = i;
                break;
            }
        }
    }
    
    if (init_index != -1) {
        print_tree(init_index, "", 1);
    } else {
        printf("No root process found\n");
    }
    
    return 0;
}
