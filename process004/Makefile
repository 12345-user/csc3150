# 定义编译器
CC = gcc
# 定义编译选项（可根据需要添加，如调试信息 -g）
CFLAGS = -Wall -Wextra

# 目标文件：与可执行文件名一致
TARGET = process004
# 源文件：与实际.c文件名一致
SRCS = process004.c

# 默认目标：编译可执行文件
all: $(TARGET)

# 编译规则：确保源文件与目标文件对应
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^

# 清理规则：删除可执行文件
clean:
	rm -f $(TARGET)

# 伪目标，防止目录中同名文件导致歧义
.PHONY: all clean