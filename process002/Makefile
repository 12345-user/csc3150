# 定义编译器
CC = gcc
# 定义编译选项（可根据需要添加，如调试信息 -g）
CFLAGS = -Wall -Wextra

# 目标文件
TARGET = process002

# 源文件
SRCS = process002.c

# 默认目标：编译可执行文件
all: $(TARGET)

# 编译规则
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^

# 清理规则：删除可执行文件
clean:
	rm -f $(TARGET)

# 伪目标，防止目录中存在同名文件时产生歧义
.PHONY: all clean