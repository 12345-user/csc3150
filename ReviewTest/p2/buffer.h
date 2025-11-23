#ifndef BUFFER_H
#define BUFFER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <unistd.h>

#define SHM_NAME "/shm_buffer"   // 共享内存名称
#define SEM_EMPTY "/sem_empty"   // 空缓冲区信号量
#define SEM_FULL "/sem_full"     // 满缓冲区信号量
#define BUFFER_SIZE 2            // 缓冲区单元数
#define ITEM_SIZE 64             // 每个单元的字节大小

// 环形缓冲区结构
typedef struct {
    char items[BUFFER_SIZE][ITEM_SIZE];
    int write_idx;
    int read_idx;
} RingBuffer;

#endif