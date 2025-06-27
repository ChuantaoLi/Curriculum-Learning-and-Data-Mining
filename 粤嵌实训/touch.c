#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <linux/input.h>

// 定义触摸点结构体
typedef struct
{
    int x;        // X坐标
    int y;        // Y坐标
    int pressure; // 触摸压力
    bool valid;   // 有效标志位
} TouchPoint;

//  结构体是一种用户定义的数据类型，它允许我们将不同数据类型的多个成员组合成一个单一的实体。
TouchPoint get_touch_data(int fd)
{
    // 定义一个input_event结构体变量ev，用于存储从触摸屏读取的数据
    struct input_event ev;
    // 定义一个TouchPoint结构体变量point，用于存储触摸点的数据
    static TouchPoint point = {0};

    // 将point的valid字段设置为false，表示数据无效
    point.valid = false;

    // 循环读取触摸屏数据
    while (read(fd, &ev, sizeof(ev)) == sizeof(ev))
    {
        // 根据ev.type的值判断数据类型
        switch (ev.type)
        {
        // 如果数据类型为EV_ABS，表示触摸点位置或压力数据
        case EV_ABS:
            // 根据ev.code的值判断具体的数据类型
            switch (ev.code)
            {
            // 如果ev.code为ABS_X，表示触摸点的X坐标
            case ABS_X:
                point.x = ev.value;
                break;
            // 如果ev.code为ABS_Y，表示触摸点的Y坐标
            case ABS_Y:
                point.y = ev.value;
                break;
            // 如果ev.code为ABS_PRESSURE，表示触摸点的压力值
            case ABS_PRESSURE:
                point.pressure = ev.value;
                break;
            }
            break;

        // 如果数据类型为EV_KEY，表示触摸键的状态
        case EV_KEY:
            // 如果ev.code为BTN_TOUCH，表示触摸键的状态
            if (ev.code == BTN_TOUCH && ev.value == 0)
            {
                // 触摸抬起时，标记数据有效
                point.valid = true;
                return point;
            }
            break;

        case EV_SYN:
            point.valid = true;
            return point;
        }
    }
    printf("Error reading touch data\n");
    return (TouchPoint){0}; // 错误或中断返回
}

int main()
{
    const char *device_path = "/dev/input/event0"; // 修改为实际设备路径
    int ts_fd = open(device_path, O_RDONLY);
    if (ts_fd == -1)
    {
        perror("Failed to open touch device");
        return 1;
    }

    printf("Touch screen device opened successfully.\n");
    printf("Start reading touch data...\n\n");

    while (1)
    {
        TouchPoint point = get_touch_data(ts_fd);

        if (point.valid)
        {
            printf("Touch Point: X=%d, Y=%d, Pressure=%d\n",
                   point.x, point.y, point.pressure);

            if (point.pressure == 0)
            {
                printf("Touch released\n");
            }
        }
    }

    close(ts_fd);
    return 0;
}
