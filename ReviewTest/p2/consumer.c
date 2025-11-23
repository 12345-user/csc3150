// 引入自定义缓冲区头文件：包含共享内存/信号量定义、环形缓冲区结构体等核心依赖
#include "buffer.h"
// 包含 strerror 函数声明（用于错误信息格式化）
#include <string.h>
// 包含 errno 定义（用于获取系统调用错误码）
#include <errno.h>

int main() {
    // 共享内存的文件描述符（类似文件句柄，用于操作共享内存对象）
    int shm_fd;
    // 指向共享内存中环形缓冲区的指针（通过该指针访问缓冲区数据）
    RingBuffer *buffer;
    // 信号量指针：sem_empty（空缓冲区计数）、sem_full（满缓冲区计数），用于进程同步
    sem_t *sem_empty, *sem_full;
    // 临时存储读取的数据（长度=单个数据单元大小，避免直接操作共享内存中的原始数据）
    char item[ITEM_SIZE];

    // ===================== 1. 打开共享内存（生产者已创建） =====================
    // shm_open：POSIX共享内存操作函数，用于打开已存在的共享内存对象
    // 参数1：SHM_NAME（从buffer.h引入，如"/shm_buffer"）- 共享内存的唯一标识
    // 参数2：O_RDWR - 打开模式为“可读可写”（需修改读指针read_idx）
    // 参数3：0666 - 权限设置（同Linux文件权限，确保进程间权限兼容）
    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    
    // 错误处理：若shm_fd == -1，说明共享内存打开失败（如生产者未启动、名称错误）
    if (shm_fd == -1) { 
        perror("shm_open consumer");  // 打印具体错误原因（如“No such file or directory”）
        exit(1);                      // 异常退出，返回错误码1
    }

    // ===================== 2. 将共享内存映射到进程地址空间 =====================
    // mmap：内存映射函数，将共享内存的物理空间映射到消费者进程的虚拟地址空间
    // 映射后，操作指针`buffer`等同于操作共享内存，无需通过read/write等文件接口
    // 参数1：0 - 让操作系统自动分配映射的起始虚拟地址（避免地址冲突）
    // 参数2：sizeof(RingBuffer) - 映射的内存大小（与环形缓冲区结构体大小一致）
    // 参数3：PROT_READ | PROT_WRITE - 映射区域的权限：允许读（读取数据）和写（修改读指针）
    // 参数4：MAP_SHARED - 共享映射模式：消费者对映射区域的修改（如read_idx）会同步到共享内存
    // 参数5：shm_fd - 共享内存的文件描述符（关联要映射的对象）
    // 参数6：0 - 映射的偏移量（从共享内存起始位置开始映射，不偏移）
    buffer = mmap(0, sizeof(RingBuffer), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    
    // 错误处理：若返回MAP_FAILED（而非NULL），说明映射失败（如权限不足、内存不足）
    if (buffer == MAP_FAILED) { 
        perror("mmap consumer");  // 打印错误原因（如“Cannot allocate memory”）
        exit(1);                  // 异常退出
    }

    // ===================== 3. 打开信号量（与生产者同步） =====================
    // 打开“空缓冲区计数”信号量（sem_empty）：仅打开已存在的信号量（生产者已创建）
    sem_empty = sem_open(SEM_EMPTY, 0);
    if (sem_empty == SEM_FAILED) { 
        perror("sem_open empty consumer"); 
        exit(1); 
    }

    // 打开“满缓冲区计数”信号量（sem_full）：仅打开已存在的信号量（生产者已创建）
    sem_full = sem_open(SEM_FULL, 0);
    if (sem_full == SEM_FAILED) { 
        perror("sem_open full consumer"); 
        exit(1); 
    }

    // ===================== 4. 从环形缓冲区读取数据（核心消费逻辑） =====================
    // 循环读取2个数据单元（与生产者写入的“Hello”“World!”数量一致）
    for (int i = 0; i < 2; i++) {
        // 步骤1：等待“满缓冲区”信号（确保有数据可读）
        // sem_wait：若sem_full值>0，将其减1后继续；若值=0，进程阻塞，直到生产者调用sem_post
        // 作用：避免消费者“读空”（读取未初始化的垃圾数据）
        sem_wait(sem_full);  

        // 步骤2：读取数据到本地变量item
        // strncpy：安全复制字符串，避免缓冲区溢出
        // 参数1：目标地址 - item（本地临时变量）
        // 参数2：源地址 - buffer->items[buffer->read_idx]（当前读指针指向的缓冲区单元）
        // 参数3：复制长度 - ITEM_SIZE（单个数据单元的大小，确保不超出范围）
        strncpy(item, buffer->items[buffer->read_idx], ITEM_SIZE);

        // 步骤3：打印读取日志，便于调试（显示读取内容和当前读指针位置）
        printf("消费者：读取 %s（读指针：%d）\n", item, buffer->read_idx);

        // 步骤4：更新读指针，实现环形循环
        // (buffer->read_idx + 1) % BUFFER_SIZE：指针+1后取模，避免越界
        // 例：BUFFER_SIZE=2时，读指针0→1→0循环，复用已读的缓冲区单元
        buffer->read_idx = (buffer->read_idx + 1) % BUFFER_SIZE;

        // 步骤5：发送“空缓冲区”信号（通知生产者有空间可写）
        // sem_post：将sem_empty值+1，若生产者因sem_empty=0（缓冲区满）阻塞，此时会被唤醒
        // 作用：同步生产者，确保消费者读取后及时释放空缓冲区供生产者写入
        sem_post(sem_empty);   
    }

    // ===================== 5. 清理资源（避免系统资源泄漏） =====================
    // 步骤1：解除共享内存映射 - 断开虚拟地址与共享内存的关联
    munmap(buffer, sizeof(RingBuffer));

    // 步骤2：关闭共享内存文件描述符 - 释放文件句柄资源
    close(shm_fd);

    // 步骤3：关闭信号量 - 释放信号量的文件描述符资源
    sem_close(sem_empty);
    sem_close(sem_full);

    // 步骤4：打印完成日志，告知用户程序正常结束
    printf("消费者：读取完成\n");

    return 0;  // 正常退出，返回0
}