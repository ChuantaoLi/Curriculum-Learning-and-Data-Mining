#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

int fd_blue;
pthread_t bule_read_ID, bule_thwrite_ID;
char recv_buf[1024]; // 此数组用于保存接受的数据

void *thread_start(void *arg);
void *thwrite_start(void *arg);
void Uart_Init(int fd);
void blue_init();
int blue_task();
