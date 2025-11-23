// 引入自定义缓冲区头文件：包含共享内存/信号量定义、环形缓冲区结构体等核心依赖
#include "buffer.h"
#include <string.h>  // 包含 strerror 函数声明
#include <errno.h>   // 包含 errno 定义

int main() {
    // 共享内存文件描述符：类似文件句柄，用于后续操作共享内存对象
    int shm_fd;
    // 指向环形缓冲区的指针：通过该指针操作共享内存中的缓冲区数据
    RingBuffer *buffer;
    // 信号量指针：sem_empty（空缓冲区计数）、sem_full（满缓冲区计数），用于进程同步
    sem_t *sem_empty, *sem_full;
    // 待写入共享内存的消息数组：对应示例中的"Hello"和"World!"，模拟生产者的输出数据
    const char *messages[] = {"Hello", "World!"};  
    // 计算消息总数：通过数组总字节数 / 单个元素字节数，避免硬编码数量
    int msg_count = sizeof(messages) / sizeof(messages[0]);

    // ===================== 1. 创建共享内存对象 =====================
    // shm_open：POSIX共享内存核心函数，创建或打开共享内存对象
    // 参数1：SHM_NAME（从buffer.h引入，如"/shm_buffer"）- 共享内存唯一标识，确保消费者能找到
    // 参数2：O_CREAT | O_RDWR - 行为标识：O_CREAT（不存在则创建）、O_RDWR（可读可写模式）
    // 参数3：0666 - 权限设置：所有用户可读可写（同Linux文件权限，避免进程间权限问题）
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    
    // 错误处理：若shm_fd == -1，说明创建失败（如权限不足、内存耗尽）
    if (shm_fd == -1) { 
        perror("shm_open");  // 打印具体错误原因（如"Permission denied"）
        exit(1);             // 异常退出，返回错误码1
    }

    // ftruncate：调整共享内存对象的大小，使其匹配环形缓冲区结构体大小
    // 参数1：shm_fd - 共享内存文件描述符，关联目标共享内存
    // 参数2：sizeof(RingBuffer) - 目标大小，确保能完整存储环形缓冲区数据
    ftruncate(shm_fd, sizeof(RingBuffer));


    // ===================== 2. 将共享内存映射到进程地址空间 =====================
    // mmap：内存映射函数，将共享内存的物理空间映射到当前进程的虚拟地址空间
    // 映射后，操作指针`buffer`等同于操作共享内存，无需通过read/write等文件接口
    // 参数1：0 - 让操作系统自动分配映射起始地址（避免手动指定导致地址冲突）
    // 参数2：sizeof(RingBuffer) - 映射内存大小（与共享内存大小一致）
    // 参数3：PROT_READ | PROT_WRITE - 映射区域权限：允许读和写（需修改write_idx指针）
    // 参数4：MAP_SHARED - 共享映射模式：当前进程对映射区的修改会同步到共享内存，供消费者感知
    // 参数5：shm_fd - 共享内存文件描述符，关联要映射的对象
    // 参数6：0 - 映射偏移量：从共享内存起始位置开始，不偏移
    buffer = mmap(0, sizeof(RingBuffer), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    // 错误处理：若返回MAP_FAILED（而非NULL），说明映射失败（如内存不足）
    if (buffer == MAP_FAILED) { 
        perror("mmap");  // 打印错误原因（如"Cannot allocate memory"）
        exit(1);         // 异常退出
    }

    // 初始化环形缓冲区的读写指针：从0开始（缓冲区初始为空，无数据可读/可写）
    buffer->write_idx = 0;  // 写指针：指向当前要写入的数据单元索引
    buffer->read_idx = 0;   // 读指针：初始化后供消费者使用，生产者暂不修改


    // ===================== 3. 创建信号量（实现进程同步） =====================
    // 信号量1：sem_empty（空缓冲区计数）- 记录当前可用的空缓冲区数量
    // sem_open：创建或打开信号量
    // 参数1：SEM_EMPTY（从buffer.h引入，如"/sem_empty"）- 信号量唯一标识
    // 参数2：O_CREAT | O_EXCL - 行为标识：O_CREAT（不存在则创建）、O_EXCL（确保是新创建，避免覆盖）
    // 参数3：0666 - 信号量权限：所有用户可操作
    // 参数4：BUFFER_SIZE - 信号量初始值：等于缓冲区单元数（初始时所有单元为空，可写）
    sem_empty = sem_open(SEM_EMPTY, O_CREAT | O_EXCL, 0666, BUFFER_SIZE);
    if (sem_empty == SEM_FAILED) {  // 错误处理：创建失败（如信号量已存在）
        perror("sem_open empty");
        exit(1);
    }

    // 信号量2：sem_full（满缓冲区计数）- 记录当前已填充数据的缓冲区数量
    // 初始值为0：缓冲区初始为空，无数据可供消费者读取
    sem_full = sem_open(SEM_FULL, O_CREAT | O_EXCL, 0666, 0);
    if (sem_full == SEM_FAILED) {  // 错误处理
        perror("sem_open full");
        exit(1);
    }


    // ===================== 4. 向环形缓冲区写入数据（核心生产逻辑） =====================
    // 循环写入所有消息：msg_count为消息总数（此处为2，对应"Hello"和"World!"）
    for (int i = 0; i < msg_count; i++) {
        // 步骤1：等待空缓冲区（确保不会写满缓冲区）
        // sem_wait：若信号量值>0，将其减1后继续；若值=0，进程阻塞，直到信号量被post（值增加）
        // 作用：避免生产者在缓冲区满时继续写入，导致数据覆盖
        sem_wait(sem_empty);  

        // 步骤2：将消息写入环形缓冲区
        // strncpy：安全复制字符串，避免缓冲区溢出
        // 参数1：目标地址 - buffer->items[buffer->write_idx]（当前写指针指向的缓冲区单元）
        // 参数2：源数据 - messages[i]（当前要写入的消息）
        // 参数3：复制长度 - ITEM_SIZE（单个缓冲区单元大小，确保不超出单元容量）
        strncpy(buffer->items[buffer->write_idx], messages[i], ITEM_SIZE);

        // 步骤3：打印写入日志，便于调试（显示写入内容和当前写指针位置）
        printf("生产者：写入 %s（写指针：%d）\n", messages[i], buffer->write_idx);

        // 步骤4：更新写指针，实现环形循环
        // (buffer->write_idx + 1) % BUFFER_SIZE：指针+1后取模，避免越界
        // 例：BUFFER_SIZE=2时，写指针0→1→0循环，覆盖旧的空缓冲区
        buffer->write_idx = (buffer->write_idx + 1) % BUFFER_SIZE;

        // 步骤5：发送"满缓冲区"信号（通知消费者有数据可读）
        // sem_post：将sem_full值+1，若消费者因sem_full=0阻塞，此时会被唤醒
        // 作用：同步消费者，确保数据写入后消费者能及时读取
        sem_post(sem_full);   
    }


    // ===================== 5. 等待消费者完成读取（可选同步逻辑） =====================
    // sleep(2)：让生产者阻塞2秒，确保消费者有足够时间读取共享内存中的数据
    // 若不等待，生产者可能提前清理共享内存，导致消费者读取失败
    // （实际项目中可通过更精准的同步机制替代，如额外信号量）
    sleep(2);  


    // ===================== 6. 清理资源（避免系统资源泄漏） =====================
    // 步骤1：解除共享内存映射 - 断开虚拟地址与共享内存的关联
    // 参数1：buffer - 映射区域的起始指针
    // 参数2：sizeof(RingBuffer) - 映射区域大小（需与mmap时一致）
    munmap(buffer, sizeof(RingBuffer));

    // 步骤2：关闭共享内存文件描述符 - 释放文件句柄资源
    close(shm_fd);

    // 步骤3：关闭信号量 - 释放信号量的文件描述符资源
    sem_close(sem_empty);
    sem_close(sem_full);

    // 步骤4：删除共享内存对象 - 彻底清理系统中的共享内存（消费者已读取完成）
    // 注意：仅生产者执行shm_unlink，避免消费者误删导致生产者操作失败
    shm_unlink(SHM_NAME);

    // 步骤5：删除信号量 - 彻底清理系统中的信号量
    sem_unlink(SEM_EMPTY);
    sem_unlink(SEM_FULL);

    // 步骤6：打印清理完成日志，告知用户程序正常结束
    printf("生产者：资源已清理\n");

    return 0;  // 正常退出，返回0
}