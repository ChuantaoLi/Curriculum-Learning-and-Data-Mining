// 蓝牙传输
#include "main.h"

int picture_blue_num = 0;
extern int picture_count;
extern Node *picture;
char blue_buf[1024];

int blue_num = 0;
// 线程函数
void *thread_start(void *arg) {
    int recv_fd_blue = *((int *)arg); // 解引用得到文件描述符

    int ret = -1;
    while (1) {
        bzero(recv_buf, sizeof(recv_buf));        // 清空
        ret = read(recv_fd_blue, recv_buf, 1024); // 读出
        fprintf(stderr, "%s", recv_buf);
        strcpy(blue_buf, recv_buf);
    }
}

void *thwrite_start(void *arg) {
    int fd_blue = *((int *)arg); // 解引用得到文件描述符
                                 // 串口1，发送数据
                                 // 数据发送
    char send_buf[64] = {0};
    char buf[64] = {0};

    while (1) {
        if (flag)
            bzero(send_buf, sizeof(send_buf)); // 清空
        bzero(buf, sizeof(buf));               // 清空
        scanf("%s", buf);                      // 从输入缓冲区中获取字符串
        sprintf(send_buf, "%s\r\n", buf);
        printf("size: %d\n", write(fd_blue, send_buf, strlen(send_buf))); // 写入send_fd_blue(文件描述符)
    }
}

// 串口初始化
void Uart_Init(int fd_blue) {
    // 定义结构体变量，保存对串口配置
    struct termios termios_new;
    bzero(&termios_new, sizeof(struct termios));

    // 将其设置成默认配置
    // 将终端设置成类似于旧版本 7 中的原始模式
    cfmakeraw(&termios_new);

    // 设置波特率 B9600
    cfsetispeed(&termios_new, B9600);
    cfsetospeed(&termios_new, B9600);

    // 本地链接和接收使能  c_cflag
    // CLOCAL Ignore modem control lines.
    // CREAD  Enable receiver.
    termios_new.c_cflag |= CLOCAL | CREAD;

    // 设置数据位8位
    // CSIZE  Character size mask.  Values are CS5, CS6, CS7, or CS8.
    termios_new.c_cflag &= ~CSIZE;
    termios_new.c_cflag |= CS8;

    // 无奇偶校验位
    // PARENB Enable parity generation on output and parity checking for input.
    termios_new.c_cflag &= ~PARENB;

    // 一停止位
    //  CSTOPB Set two stop bits, rather than one.
    termios_new.c_cflag &= ~CSTOPB;

    // 设置接收字符以及等待时间
    termios_new.c_cc[VTIME] = 10;
    termios_new.c_cc[VMIN] = 1;

    // 清空缓冲区
    // TCIFLUSH flushes data received but not read.
    tcflush(fd_blue, TCIFLUSH);

    // 激活串口配置
    // TCSANOW Wthe change occurs immediately. 改变立即生效
    if (tcsetattr(fd_blue, TCSANOW, &termios_new)) {
        perror("tcsetattr error");
    } else {
        printf("tcsetattr\n");
    }
}

void blue_init() {
    // 1. 打开串口 /dev/ttySAC1
    // O_NOCTTY    ---> 告诉系统该程序不成为端口的控制端
    // O_NONBLOCK  ---> 以不可阻断方式打开端口
    fd_blue = open("/dev/ttySAC1", O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (-1 == fd_blue) // 出错处理
    {
        perror("tty open error"); // 专门用来输出函数的出错信息的
        return ;
    } else {
        printf("蓝牙打开成功\n");
    }

    // 1. 串口初始化
    Uart_Init(fd_blue);

    // 数据收发。串口一发送数据，串口二接收数据
    // 开启线程，使串口能够同时进行收发数据。
    pthread_create(&bule_read_ID, NULL, thread_start, (void *)&fd_blue);
    pthread_create(&bule_thwrite_ID, NULL, thwrite_start, (void *)&fd_blue);
}

int blue_task() {

    if (blue_num == 0) {
        display_picture(picture, picture_blue_num);
        blue_num = 1;
    }

    if (strcmp(blue_buf, "next") == 0) // 通过蓝牙发送的数据来控制切换图片//下一张
    {
        picture_blue_num++;
        if (picture_blue_num >= picture_count) {
            picture_blue_num = 0;
        }
        printf("下一张:%d\n", picture_blue_num);
        display_picture(picture, picture_blue_num);

    } else if (strcmp(blue_buf, "pre") == 0) // 上一张
    {

        picture_blue_num--;
        if (picture_blue_num < 0) {
            picture_blue_num = picture_count - 1;
        }
        printf("上一张:%d\n", picture_blue_num);
        display_picture(picture, picture_blue_num);

    } else if (strcmp(blue_buf, "exit") == 0) // 退出
    {
        printf("退出\n");
        return 0;
    }

    return 3;
}
