# 定义变量，方便修改和维护
CC = gcc
CFLAGS = -Wall -g
TARGET = father_son_fork_test

# 默认目标，执行 make 时会编译所有
all: $(TARGET)

# 生成可执行文件
$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

# 清理编译生成的文件
.PHONY: clean
clean:
	rm -f $(TARGET)