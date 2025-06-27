#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include "jpeglib.h"
#include "setjmp.h"
#include <string.h>

int *lcd_ptr;
int lcd_fd, ts_fd;

// 自定义libjpeg库的错误处理机制。libjpeg在遇到错误时，默认行为是打印错误信息并直接退出程序。
// 通过自定义错误处理，可以捕获JPEG错误，避免程序崩溃，并进行资源清理。
struct my_error_mgr
{
	struct jpeg_error_mgr pub; /* "public" fields */

	jmp_buf setjmp_buffer; /* for return to caller */
};

typedef struct my_error_mgr *my_error_ptr;

METHODDEF(void)
my_error_exit(j_common_ptr cinfo)
{
	// 强制类型转换为我们自定义的my_error_mgr指针，以便访问setjmp_buffer。
	my_error_ptr myerr = (my_error_ptr)cinfo->err;

	// 调用libjpeg默认的错误消息输出函数，将错误信息打印到标准错误流（stderr）。
	(*cinfo->err->output_message)(cinfo);

	// 它执行一个非局部跳转，直接跳回到read_JPEG_file函数中setjmp(jerr.setjmp_buffer)的位置。
	longjmp(myerr->setjmp_buffer, 1);
}

#pragma pack(1) // 设置1字节对齐模式，pack()将对齐模式取消
/*位图文件头*/
typedef struct BMP_FILE_HEADER
{
	uint16_t bType;		 /*  文件标识符          */
	uint32_t bfSize;	 /*  文件的大小，单位字节 */
	uint16_t bReserved1; /*  保留值,必须设置为0  */
	uint16_t bReserved2; /*  保留值,必须设置为0  */
	uint32_t bOffset;	 /*  文件头的最后到图像数据位开始的偏移量    */
} BMPFILEHEADER;		 // 14字节

/*位图信息头*/
typedef struct BMP_INFO
{
	uint32_t bInfoSize;		 /*  信息头的大小             */
	uint32_t bWidth;		 /*  图像的宽度               */
	uint32_t bHeight;		 /*  图像的高度               */
	uint16_t bPlanes;		 /*  图像的位面数             */
	uint16_t bBitCount;		 /*  每个像素的位数           */
	uint32_t bCompression;	 /*  压缩类型                 */
	uint32_t bmpImageSize;	 /*  图像的大小,以字节为单位   */
	uint32_t bXPelsPerMeter; /*  水平分辨率               */
	uint32_t bYPelsPerMeter; /*  垂直分辨率               */
	uint32_t bClrUsed;		 /*  使用的色彩数             */
	uint32_t bClrImportant;	 /*  重要的颜色数             */
} BMPINF;					 // 40字节

#pragma pack()
/*彩色表*/
typedef struct RGB_QUAD
{
	uint16_t rgbBlue;	  /*  蓝色强度  */
	uint16_t rgbGreen;	  /*  绿色强度  */
	uint16_t rgbRed;	  /*  红色强度  */
	uint16_t rgbReversed; /*  保留值    */
} RGBQUAD;

// 在LCD屏幕的指定坐标 (i, j) 绘制一个指定颜色的像素点。
int lcd_draw_point(int i, int j, int color)
{
	*(lcd_ptr + 800 * j + i) = color;
	// lcd_ptr：指向LCD帧缓冲内存基地址的全局指针。
}

//  打开、读取并显示一个24位的BMP图片文件到LCD屏幕的指定位置 (x, y)。
int lcd_draw_bmp(const char *pathname, int x, int y)
{
	int i, j;

	// 使用open函数以读写模式打开pathname指定的BMP文件。进行错误检查。
	int bmp_fd = open(pathname, O_RDWR);

	// 错误处理
	if (bmp_fd == -1)
	{
		printf("open bmp file failed!\n");
		return -1;
	}

	BMPFILEHEADER file_head; // 位图文件头
	BMPINF info_head;		 // 位图信息头

	// 读取图片信息头，得到图片的相关信息
	read(bmp_fd, &file_head, sizeof(file_head));
	read(bmp_fd, &info_head, sizeof(info_head));

	int w = info_head.bWidth;
	int h = info_head.bHeight;

	char rgb_buf[w * h * 3];
	// 声明一个局部数组char rgb_buf[w * h * 3]，用于存储BMP的像素数据。
	// 此处的w * h * 3表示每个像素3字节（24位BMP）。

	int pad = (4 - (w * 3) % 4) % 4;
	// 计算每一行像素数据末尾的填充字节数。BMP文件为了对齐，每行数据长度通常是4字节的倍数。

	for (i = 0; i < h; i++)
	{
		// 读取图片颜色数据
		read(bmp_fd, &rgb_buf[w * i * 3], w * 3);

		// 跳过当前行末尾的填充字节，以便读取下一行
		lseek(bmp_fd, pad, SEEK_CUR);
	}

	for (j = 0; j < h; j++)
	{
		for (i = 0; i < w; i++)
		{
			// BMP图片以BGR（Blue-Green-Red）顺序存储像素。代码从rgb_buf中读取b、g、r分量。
			int b = rgb_buf[(j * w + i) * 3 + 0];
			int g = rgb_buf[(j * w + i) * 3 + 1];
			int r = rgb_buf[(j * w + i) * 3 + 2];

			// 将BGR分量转换为32位的ARGB格式。
			int color = b;
			color |= (g << 8);
			color |= (r << 16);

			// 调用lcd_draw_point函数绘制像素。
			// h - 1 - j 是垂直翻转操作
			lcd_draw_point(i + x, h - 1 - j + y, color);
		}
	}

	close(bmp_fd);

	return 0;
}

// 打开、读取、缩放并显示一个24位BMP图片文件到LCD屏幕的指定位置 (x, y) 和指定大小 (w, h)。
int lcd_draw_scale_bmp(const char *pathname, int x, int y, int w, int h)
{
	int i, j;

	// 打开图片文件
	int bmp_fd = open(pathname, O_RDWR);

	// 错误处理
	if (bmp_fd == -1)
	{
		printf("open bmp file failed!\n");
		return -1;
	}

	// 将图片数据加载到lcd屏幕

	char header[54];
	char rgb_buf[w * h * 3];

	// 将图片颜色数据读取出来
	read(bmp_fd, header, 54);

	// 获取 bmp 图片的宽度，高度，位深度，大小
	int bmp_w = *(int *)&header[18];
	int bmp_h = *(int *)&header[22];
	int bmp_bit = *(short *)&header[28];
	int bmp_size = *(int *)&header[34];

	// //计算宽度缩放比例和高度缩放比例
	int scale_w = bmp_w / w;
	int scale_h = bmp_h / h;

	// 分配存储bmp图片数据的内存空间
	char *bmp_buf = (char *)calloc(1, bmp_size);
	// 分配存储缩放后图片数据的内存空间
	char *scale_buf = (char *)calloc(1, w * h * 3);

	// 读取 bmp 图片数据
	int pad = (4 - (bmp_w * 3) % 4) % 4; // 计算每一行的无效字节数

	for (i = 0; i < bmp_h; i++)
	{
		// 读取图片颜色数据
		read(bmp_fd, &bmp_buf[bmp_w * i * 3], bmp_w * 3);
		// 跳过无效字节
		lseek(bmp_fd, pad, SEEK_CUR);
	}

	// 对缩放后的每一个像素点进行计算
	for (j = 0; j < h; j++)
	{
		for (i = 0; i < w; i++)
		{
			int k, l;
			int b = 0, g = 0, r = 0;
			for (k = 0; k < scale_h; k++)
			{
				for (l = 0; l < scale_w; l++)
				{
					b += bmp_buf[((bmp_h - 1 - j * scale_h - k) * bmp_w + i * scale_w + l) * 3 + 0];
					g += bmp_buf[((bmp_h - 1 - j * scale_h - k) * bmp_w + i * scale_w + l) * 3 + 1];
					r += bmp_buf[((bmp_h - 1 - j * scale_h - k) * bmp_w + i * scale_w + l) * 3 + 2];
				}
			}
			b /= (scale_w * scale_h);
			g /= (scale_w * scale_h);
			r /= (scale_w * scale_h);

			scale_buf[(j * w + i) * 3 + 0] = b;
			scale_buf[(j * w + i) * 3 + 1] = g;
			scale_buf[(j * w + i) * 3 + 2] = r;
		}
	}

	// 将缩放后的数据加载到LCD屏幕
	for (j = 0; j < h; j++)
	{
		for (i = 0; i < w; i++)
		{
			int b = scale_buf[(j * w + i) * 3 + 0];
			int g = scale_buf[(j * w + i) * 3 + 1];
			int r = scale_buf[(j * w + i) * 3 + 2];

			int color = b;
			color |= (g << 8);
			color |= (r << 16);

			lcd_draw_point(i + x, j + y, color);
		}
	}

	// 3，释放资源
	free(bmp_buf);
	free(scale_buf);

	// 关闭图片文件
	close(bmp_fd);

	return 0;
}

GLOBAL(int)
// 使用libjpeg库解压缩指定的JPEG文件，并返回解压后的原始像素数据以及图像的宽度和高度。
// 此函数包含了自定义的错误处理机制。
read_JPEG_file(const char *filename, int *image_width, int *image_height, char **out_buffer)
{

	struct jpeg_decompress_struct cinfo;

	struct my_error_mgr jerr;
	FILE *infile;
	JSAMPARRAY buffer;
	int row_stride;

	// 以二进制读取模式打开JPEG文件。进行错误检查。
	if ((infile = fopen(filename, "rb")) == NULL)
	{
		fprintf(stderr, "can't open %s\n", filename);
		return 0;
	}

	/* Step 1: 分配并初始化解压缩对象 */
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;

	if (setjmp(jerr.setjmp_buffer))
	{
		// 如果libjpeg在后续操作中遇到致命错误并调用了my_error_exit，my_error_exit中的longjmp会跳回这里，
		// 此时setjmp的返回值为非零（通常是longjmp传入的参数1），从而进入错误清理逻辑。
		jpeg_destroy_decompress(&cinfo);
		fclose(infile);
		return 0;
	}
	jpeg_create_decompress(&cinfo);

	/* Step 2: 指定解压缩数据源 */
	jpeg_stdio_src(&cinfo, infile);

	/* Step 3: 读取文件头数据 */
	(void)jpeg_read_header(&cinfo, TRUE);
	*image_width = cinfo.image_width;
	*image_height = cinfo.image_height;

	/* Step 4: 设置解压参数，可以直接使用默认的解压缩参数 */

	/* Step 5: 开始解压 */
	(void)jpeg_start_decompress(&cinfo);

	/* 一行的数据大小 */
	row_stride = cinfo.output_width * cinfo.output_components;

	*out_buffer = calloc(1, row_stride * cinfo.image_height);

	/* 一行数据的存储缓冲区 */
	buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

	char *dst = *out_buffer;

	/* Step 6: 循环读取每一行数据 */
	while (cinfo.output_scanline < cinfo.output_height)
	{
		(void)jpeg_read_scanlines(&cinfo, buffer, 1);

		memcpy(dst, buffer[0], row_stride);
		dst += row_stride;
	}

	/* Step 7: 解压完成 */
	(void)jpeg_finish_decompress(&cinfo);

	/* Step 8: 释放资源 */
	jpeg_destroy_decompress(&cinfo);
	fclose(infile);

	return 1;
}

int width, height;

void jpeg_to_lcd(int x, int y, char *jpeg_buffer)
{
	int i, j;

	int b, g, r, color;

	for (j = 0; j < height; j++)
	{
		for (i = 0; i < width; i++)
		{
			r = jpeg_buffer[(width * j + i) * 3 + 0];
			g = jpeg_buffer[(width * j + i) * 3 + 1];
			b = jpeg_buffer[(width * j + i) * 3 + 2];
			color = r << 16 | g << 8 | b;
			lcd_ptr[800 * j + i] = color;
		}
	}
}

int lcd_draw_jpeg(int x, int y, const char *pathname)
{

	char *jpeg_buffer = NULL;

	// 1. 解压
	read_JPEG_file(pathname, &width, &height, &jpeg_buffer);
	// 2. 显示
	jpeg_to_lcd(x, y, jpeg_buffer);
}

int dev_init()
{
	// 1 打开设备文件

	lcd_fd = open("/dev/fb0", O_RDWR);

	// 错误处理
	if (lcd_fd == -1)
	{
		printf("open lcd device failed!\n");
		return -1;
	}

	// 2 为lcd设备建立内存映射关系
	lcd_ptr = mmap(NULL, 800 * 480 * 4, PROT_READ | PROT_WRITE, MAP_SHARED, lcd_fd, 0);

	if (lcd_ptr == MAP_FAILED)
	{
		printf("mmap failed!\n");
		return -1;
	}

	ts_fd = open("/dev/input/event0", O_RDWR);

	// 错误处理
	if (ts_fd == -1)
	{
		printf("open ts device failed!\n");
		return -1;
	}

	return 0;
}

int dev_uninit()
{
	munmap(lcd_ptr, 800 * 480 * 4);
	// 关闭设备文件
	close(lcd_fd);

	close(ts_fd);
}

int main(void)
{

	dev_init();

	lcd_draw_jpeg(0, 0, "./01.jpg");

	dev_uninit();

	return 0;
}