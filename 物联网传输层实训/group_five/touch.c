#include "main.h"
#include <fcntl.h>
#include <linux/input.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

// 全局变量
int touch_file;
int touch_x = 0, touch_y = 0;
struct input_event touch;
pthread_t touch_pid;

// 触摸初始化
int touch_init() {
    touch_file = open("/dev/input/event0", O_RDONLY);
    if (touch_file == -1) {
        printf("打开触摸文件失败\n");
        return -1;
    } else {
        printf("打开触摸文件成功\n");
    }

    pthread_create(&touch_pid, NULL, touch_task, NULL);
    return 0;
}

// 读取触摸事件
int touch_read() {
    if (read(touch_file, &touch, sizeof(touch)) == -1) {
        perror("读取触摸事件失败");
        return -1;
    }

    if (touch.type == EV_ABS && touch.code == ABS_X) {
        touch_x = touch.value * 800 / 1024; // 根据屏幕宽度和实际范围调整
    }
    if (touch.type == EV_ABS && touch.code == ABS_Y) {
        touch_y = touch.value * 480 / 600; // 根据屏幕高度和实际范围调整
    }

    if (touch.type == EV_KEY && touch.code == BTN_TOUCH && touch.value == 0) {
        printf("抬起检测: x=%d, y=%d\n", touch_x, touch_y);
    }

    return 0;
}

// 获取触摸功能标志
int touch_get_flag() {
    int touch_flag = 0;

    if (touch_x > 0 && touch_x < 400 && touch_y > 0 && touch_y < 240) // 点击app 电子相册
    {
        touch_flag = 1; // 多人聊天室
        printf("多人聊天室\n");
    } else if (touch_x > 0 && touch_x < 400 && touch_y > 240 && touch_y < 480) // 点击app 音乐播放
    {
        touch_flag = 2; // 图片上传
        printf("图片上传\n");
    } else if (touch_x > 400 && touch_x < 700 && touch_y > 0 && touch_y < 240) {
        touch_flag = 3; // 蓝牙
        printf("蓝牙\n");
    }

    return touch_flag;
}

// 关闭触摸设备
void touch_close() {
    close(touch_file);
    printf("关闭触摸文件成功\n");
}

// 触摸任务线程
void *touch_task(void *arg) {
    while (1) {
        touch_read();
        usleep(1000); // 延时减少 CPU 占用
    }
    return NULL;
}
