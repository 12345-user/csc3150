# 定义可执行文件的名称
TARGET = signal02

# 定义源文件
SRC = signal02.c

# 定义编译器
CC = gcc

# 定义编译选项
# -Wall: 开启所有警告信息，有助于发现潜在问题
# -g: 生成调试信息，方便使用 gdb 进行调试
CFLAGS = -Wall -g

# 默认目标，执行 make 时会首先执行这个目标
all: $(TARGET)
	@echo "编译完成！可执行文件为: $(TARGET)"

# 链接目标，定义了如何从源文件生成可执行文件
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $^ -o $@

# 运行目标，用于直接运行编译后的程序
# 使用 .PHONY 声明 run 是一个伪目标，不是一个文件
.PHONY: run
run: $(TARGET)
	@echo "正在运行程序: ./"$(TARGET)
	./$(TARGET)

# 清理目标，用于删除编译生成的文件
.PHONY: clean
clean:
	rm -f $(TARGET)
	@echo "清理完成！已删除可执行文件: $(TARGET)"

# 帮助目标，用于显示 Makefile 的使用说明
.PHONY: help
help:
	@echo "Makefile for $(TARGET)"
	@echo "用法:"
	@echo "  make [目标]"
	@echo ""
	@echo "目标列表:"
	@echo "  all       (默认) 编译源文件生成可执行文件"
	@echo "  run       编译并运行程序"
	@echo "  clean     删除生成的可执行文件"
	@echo "  help      显示此帮助信息"