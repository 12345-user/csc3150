# SC3150 Assignment 1 - Bonus Task

## 📋 Bonus Task 实现说明

本目录包含SC3150 Assignment 1的Bonus Task实现，主要是自定义的`pstree`命令实现。

## 🎯 实现的功能

### pstree.c
- ✅ 基本进程树显示 (5分)
- ✅ 支持 `-p` 选项显示PID (1分)
- ✅ 支持 `-h` 选项显示帮助 (1分)
- ✅ 支持 `-V` 选项显示版本 (1分)
- ✅ 支持 `-n` 选项数字排序 (1分)
- ✅ 支持 `-T` 选项隐藏线程 (1分)

### myfork.c
- 演示fork功能的简单程序

## 🚀 编译和运行

```bash
# 编译所有程序
make

# 或者编译单个程序
make pstree
make myfork

# 运行pstree
./pstree

# 显示带PID的进程树
./pstree -p

# 显示帮助
./pstree -h

# 显示版本
./pstree -V

# 运行fork演示
./myfork

# 清理编译文件
make clean
```

## 📊 预期输出示例

### 基本进程树
```
systemd─┬─NetworkManager───2*[{NetworkManager}]
        ├─abrt-watch-log
        ├─abrtd
        ├─agetty
        ├─atd
        └─gnome-terminal-───bash───pstree
```

### 带PID的进程树
```
systemd(1)─┬─NetworkManager(756)───2*[{NetworkManager}(758,759)]
           ├─abrt-watch-log(712)
           ├─abrtd(711)
           └─gnome-terminal-(2456)───bash(2467)───pstree(3421)
```

## 🎁 评分标准

- 基本进程树显示: 5分
- 每个额外选项: 1分 × 5 = 5分
- 总计: 10分

## 📝 技术实现

- 使用 `/proc` 文件系统读取进程信息
- 解析 `/proc/pid/stat` 获取进程名和父进程ID
- 构建进程树数据结构
- 递归打印进程树
- 支持多种命令行选项

## ⚠️ 注意事项

- 需要在Linux系统上运行
- 需要访问 `/proc` 文件系统
- 某些进程可能因权限问题无法访问
