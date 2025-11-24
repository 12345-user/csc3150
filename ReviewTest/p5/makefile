# 定义可执行文件的名称
TARGET = omp_demo_no_macro

# 定义源文件
SRC = omp_demo_no_macro.c

# 定义编译器
CC = gcc

# 定义编译选项
# -fopenmp: 启用 OpenMP 支持
# -Wall: 开启所有警告信息
# -O2: 启用优化级别 2
CFLAGS = -fopenmp -Wall -O2

# 默认目标
all: $(TARGET)

# 链接目标
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $^ -o $@
	@echo "编译成功！可执行文件为: $(TARGET)"

# 清理目标
.PHONY: clean
clean:
	rm -f $(TARGET)
	@echo "清理完成！"