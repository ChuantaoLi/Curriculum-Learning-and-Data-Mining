#!/bin/bash

# 定义编译器
CC=arm-linux-gcc

# 定义编译选项，包括 C99 标准和链接 pthread 库
CFLAGS="-std=c99 -lpthread"

# 定义源文件列表
# 注意：这里假设所有的 .h 文件都是必要的，并且所有的 .c 文件都需要被编译。
# 如果有特定的文件不需要编译或有其他的编译规则，需要相应地调整这个数组。
SRCS=("blue.c" "client_file.c" "LCD.c" "main.c" "tcp_broadcast.c" "touch.c" )
#SRCS=("touch.c" "LCD.c" "main.c" "bule.c")
# 定义输出的可执行文件名
OUTPUT="newProject"

# 编译所有源文件并链接生成可执行文件
$CC $CFLAGS "${SRCS[@]}" -o $OUTPUT

# 检查编译是否成功
if [ $? -eq 0 ]; then
    echo "Compilation successful. Output: $OUTPUT"
else
    echo "Compilation failed."
fi
