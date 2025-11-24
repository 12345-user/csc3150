#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void sigterm_handler(int signum) {
    printf("Received SIGTERM (signal %d). Handling...\n", signum);
    sleep(2); // 模拟处理耗时操作（期间屏蔽 SIGINT）
    printf("SIGTERM handling done. Exiting...\n");
    _exit(0);
}

int main() {
    //sigaction是用来注册信号处理函数的结构体
    struct sigaction act;
    // 初始化信号处理配置
    //sa_handler是信号处理函数指针，指向处理该信号的函数
    act.sa_handler = sigterm_handler; // 绑定处理函数
    //sigemptyset初始化信号掩码为空，表示在处理该信号时不屏蔽任何信号
    //信号掩码是在处理某个信号时临时屏蔽掉其他信号，防止它们打断当前的信号处理过程
    sigemptyset(&act.sa_mask); // 初始化信号掩码为空
    //sigaddset向信号掩码中添加指定信号,这里添加SIGINT信号,意思是在处理SIGTERM时屏蔽SIGINT信号
    sigaddset(&act.sa_mask, SIGINT); // 处理 SIGTERM 时，屏蔽 SIGINT
    //sa_flags是一些标志位，用于控制信号处理的行为
    //SA_RESTART表示如果信号处理函数返回后，之前被信号中断的系统调用会自动重启
    act.sa_flags = SA_RESTART; // 重启被信号中断的系统调用（如 sleep()）

    // 注册 SIGTERM 信号
    //sigaction函数用于改变信号的处理方式，这里将SIGTERM信号与上面配置的处理方式绑定
    //sigaction函数返回-1表示注册失败，成功返回0
    //将act结构体中的配置应用到SIGTERM信号上
    //null表示不需要保存旧的信号处理方式
    if (sigaction(SIGTERM, &act, NULL) == -1) {
        perror("sigaction failed");
        return 1;
    }

    // 主程序等待信号,打印等待信息
    printf("Waiting for SIGTERM (send with: kill %d)...\n", getpid());
    while (1) {
        sleep(1);
    }

    return 0;
}