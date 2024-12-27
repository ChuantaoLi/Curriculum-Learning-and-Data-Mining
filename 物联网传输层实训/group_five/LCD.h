#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define WIDTH 800
#define HEIGH 480

#define MAX_FILENAME_LENGTH 256

typedef struct Node {
    char filename[MAX_FILENAME_LENGTH];
    struct Node *next;
} Node; // 定义一个链表

Node *picture; // 定义一个全局变量，存放图片链表
Node *UI_picture;

int picture_count; // 定义一个全局变量，存放图片数量
int ui_picture_count;

Node *initList();                                   // 初始化链表
void addNode(Node **head, const char *filename);    // 添加一个文件
void deleteNode(Node **head, const char *filename); // 删除一个文件
int findNode(Node *head, const char *filename);     // 查找文件/输入文件名字查找
int printList(Node *head);                          // 打印所有图片名字测试使用,并且返回图片数量
char *getNodeAtIndex(Node *head, int index);        // 查找第index个文件，索引第一个是0
void LCD_desktop();

void rot();
void LCD_init();                        // LCD屏幕初始换
int display_picture(Node *head, int i); // 显示图片，输入文件标号
int LCD_task();                         // 图片显示任务
