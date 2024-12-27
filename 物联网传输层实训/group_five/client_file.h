#include <arpa/inet.h>
#include <pthread.h> // 引入 POSIX 线程库
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_BUF_SIZE 1024

void client_file_init();
void *send_data(void *arg);
void *receive_data(void *arg);
