# 定义目标文件和可执行文件名
TARGET = thread
SRC = thread.c

# 编译选项
CFLAGS = -Wall -std=c99 -g
# 链接线程库（关键：添加 -lpthread）
LDFLAGS = -lpthread

# 默认目标：编译生成可执行文件
all: $(TARGET)

# 可执行文件依赖于源码文件，编译命令
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

# 清洁目标：删除可执行文件
clean:
	rm -f $(TARGET)