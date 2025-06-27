#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <stdlib.h>
#include "bmp.h"

#define LCD_NAME "/dev/fb0"
#define WIDTH 800
#define HEIGH 480

int lcd_fd;                 // LCD文件描述符
unsigned int width, height; // LCD的宽和高
unsigned int screen_size;   // LCD大小
unsigned int *lcd_fp;       // LCD映射后的显存基地址
// 描述LCD信息的结构体，这两个结构体在fb.h里面
struct fb_fix_screeninfo fb_fix;
struct fb_var_screeninfo fb_var;
int x_pos = 0, y_pos = 0;

// 函数：lcd_draw_point
// 功能：在LCD屏幕上绘制一个点
// 参数：x - 点的横坐标，y - 点的纵坐标，color - 点的颜色
void lcd_draw_point(int x, int y, unsigned int color)
{
    // 将颜色值写入LCD屏幕的指定位置
    *(lcd_fp + y * 1024 + x) = color;
    // lcd_fp: 全局变量，LCD 显存的基地址。
}

// 将输入的图像数据（一维数组表示）逆时针旋转 90 度，并存储到目标数组中。
void rotate(int src[], int dest[], int width, int height)
{
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            dest[i + width * j] = src[i + width * (height - j - 1)];
        }
    }
}

// 打开并读取一个名为 "1.bmp" 的 BMP 图片文件，将其数据转换为 ARGB 格式，进行垂直翻转，然后显示到 /dev/fb0 帧缓冲设备上。
int display_picture()
{
    // char *buff = getNodeAtIndex(head, i); // 查找第index个文件，索引第一个是0
    int fb0 = open("/dev/fb0", O_RDWR);
    int bmp = open("1.bmp", O_RDWR);
    int *plcd = NULL;

    if (fb0 == -1 || bmp == -1)
    {
        // printf("open lcd failed\n");
        return -1;
    }

    // mmap the framebuffer device
    plcd = (int *)mmap(NULL, WIDTH * HEIGH * 4, PROT_READ | PROT_WRITE, MAP_SHARED, fb0, 0);
    if (plcd == MAP_FAILED)
    {
        close(fb0);
        close(bmp);
        return -1;
    }

    char buf[WIDTH * HEIGH * 3];
    lseek(bmp, 54, SEEK_SET);
    // 使用 lseek 将文件指针移动到 BMP 文件的第 54 个字节，
    // 跳过 BMP 文件头和信息头，直接定位到像素数据开始的位置。

    int rd = read(bmp, buf, WIDTH * HEIGH * 3);
    // 使用 read 读取 WIDTH * HEIGH * 3 字节的 BMP 像素数据到 buf 数组
    // 这里假设 BMP 是 24 位（RGB）格式。

    int argb[WIDTH * HEIGH] = {0};
    for (int i = 0; i < WIDTH * HEIGH; i++)
    {
        argb[i] = (buf[3 * i + 2] << 16) | (buf[3 * i + 1] << 8) | buf[3 * i];
    }
    // 遍历 buf 中的 RGB 数据，将其转换为 32 位 ARGB 格式并存储到 argb 数组。
    // 每个像素的 ARGB 值为 (buf[3 * i + 2] << 16) | (buf[3 * i + 1] << 8) | buf[3 * i]，
    // 这对应的是 BGR 顺序（BMP 文件通常是 BGR 存储）。

    // Rotate the image
    int argb_rotated[WIDTH * HEIGH] = {0};
    rotate(argb, argb_rotated, WIDTH, HEIGH);
    // 调用 rotate 函数，将 argb 数组中的数据垂直翻转后存储到 argb_rotated 数组中。

    // Copy rotated image to framebuffer
    for (int i = 0; i < WIDTH * HEIGH; i++)
    {
        plcd[i] = argb_rotated[i];
    }
    // 将 argb_rotated 数组中的数据逐像素拷贝到 plcd 指向的帧缓冲显存中，从而在屏幕上显示图片。

    // Unmap and close files
    munmap(plcd, WIDTH * HEIGH * 4);
    close(fb0);
    close(bmp);
    return 0;
}

// 初始化 LCD 帧缓冲设备，包括打开设备文件、获取屏幕参数以及将显存映射到用户空间。
int lcd_init(void)
{

    /*1.调用open()打开/dev/fbX 设备文件*/
    lcd_fd = open("/dev/fb0", O_RDWR);

    // 判断是否打开失败
    if (lcd_fd == -1)
    {
        // 将失败的原因打印出来
        perror("open lcd");
        return -1;
    }

    /*2.使用 ioctl()函数获取到当前显示设备的参数信息，譬如屏幕的分辨率大小、
        像素格式，根据屏幕参数计算显示缓冲区的大小。*/
    ioctl(lcd_fd, FBIOGET_VSCREENINFO, &fb_var); // 帧缓冲设备的可变参数（分辨率、像素格式等）
    ioctl(lcd_fd, FBIOGET_FSCREENINFO, &fb_fix); // 固定参数（行长度等）

    screen_size = fb_fix.line_length * fb_var.yres; // 显示屏的大小
    width = fb_var.xres;                            // 显示屏的宽
    height = fb_var.yres;                           // 显示屏的高
    printf("显示屏的大小是%d\n", screen_size);
    printf("显示屏的宽是%d,高是%d\n", width, height);
    printf("显示屏的像素点大小是%d字节\n", fb_var.bits_per_pixel / 8);

    /*3.通过存储映射 I/O 方式将屏幕的显示缓冲区映射到用户空间*/
    lcd_fp = mmap(NULL, screen_size, PROT_WRITE, MAP_SHARED, lcd_fd, 0); // 程序就可以直接通过 lcd_fp 指针来读写显存
    if (MAP_FAILED == (void *)lcd_fp)
    {
        perror("mmap error");
        close(lcd_fd);
        return -1;
    }
}

void lcd_close(void)
{
    /*5.调用 munmap()取消映射、并调用 close()关闭设备文件*/
    munmap(lcd_fp, screen_size);
    close(lcd_fd);
    return;
}

int main()
{
    int ret = lcd_init(); // 调用 lcd_init() 函数初始化 LCD
    if (ret == -1)
    {
        printf("显示屏初始化失败!\n");
        return -1;
    }

    ret = display_picture(); // 调用 display_picture() 函数显示图片
    if (ret != 0)
    {
        printf("图片显示失败!\n");
    }
    printf("图片显示成功\n");

    lcd_close();

    return 0;
}
