#include <stdio.h>
#include <signal.h>
#include <unistd.h>

// 信号处理函数：收到 SIGINT 后打印信息并退出
void sigint_handler(int signum) {
    printf("\nReceived SIGINT (signal %d). Exiting...\n", signum);
    _exit(0); // 强制退出（避免缓冲区未刷新）
}

int main() {
    // 注册 SIGINT 信号处理函数,其中sigint是信号编号，sigint_handler是处理函数
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        perror("signal failed");
        return 1;
    }

    printf("Waiting for SIGINT (press Ctrl+C)...\n");
    while (1) {
        sleep(1); // 无限循环，等待信号
    }

    return 0;
}