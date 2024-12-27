// 触摸板反应
#include <fcntl.h>
#include <linux/input.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

struct input_event touch;
int touch_x, touch_y;
int touch_file;
pthread_t touch_pid; // 触摸屏线程
int touch_get_flag();
int touch_init();            // 触摸屏初始化
int touch_read();            // 读取触摸屏数据
void touch_close();          // 关闭触摸屏
void *touch_task(void *arg); // 触摸屏线程
