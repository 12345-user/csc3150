# 定义变量，方便后续修改和维护
# 编译器
CC = gcc

# 可执行文件名称
TARGET = TS

# 源文件列表
SRCS = TS.c

# 目标文件列表，将源文件的 .c 后缀替换为 .o
OBJS = $(SRCS:.c=.o)

# 编译选项
# -Wall: 开启所有警告信息，有助于写出更规范的代码
# -Wextra: 开启额外的警告
# -g: 生成调试信息，方便使用 gdb 进行调试
# -pthread: 链接 pthread 库，这是使用 POSIX 线程所必需的
CFLAGS = -Wall -Wextra -g -pthread

# 链接选项
# 同样需要 -pthread 来确保正确链接线程库
LDFLAGS = -pthread

# 默认目标，当直接输入 make 时执行
all: $(TARGET)

# 链接目标文件，生成最终的可执行文件
# $@ 代表目标文件 (thread_scope_demo)
# $^ 代表所有依赖文件 (thread_scope_demo.o)
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo "编译成功！可执行文件为: $(TARGET)"

# 编译源文件，生成目标文件
# $@ 代表目标文件 (thread_scope_demo.o)
# $< 代表第一个依赖文件 (thread_scope_demo.c)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 伪目标：清理编译生成的文件
# .PHONY 用于声明 clean 是一个伪目标，而非实际的文件
.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)
	@echo "清理完毕！"

# 伪目标：运行程序
.PHONY: run
run: $(TARGET)
	./$(TARGET)

# 伪目标：使用 gdb 调试程序
.PHONY: debug
debug: $(TARGET)
	gdb ./$(TARGET)