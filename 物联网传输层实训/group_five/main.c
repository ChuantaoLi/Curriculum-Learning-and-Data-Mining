#include "main.h"
#include <stdio.h>

// 全局变量
int flag;
int blue_num;

// 主程序入口
int main(void) {
    // 初始化
    LCD_init();
    touch_init();
    blue_init();
    tcp_broadcast_init();

    flag = 0;

    while (1) {
        if (flag == 0) // UI界面选择
        {
            LCD_desktop();
            flag = touch_get_flag();
            blue_num = 0;
        } else if (flag == 1) // 多人聊天室
        {
            UI_broadcast();
            flag = broadcast_task();
        } else if (flag == 2) // 相册
        {
            flag = LCD_task();
        } else if (flag == 3) // 蓝牙图片传输
        {
            flag = blue_task();
        }
    }

    return 0;
}
