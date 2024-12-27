#include <arpa/inet.h> //提供专属结构体的头文件和 htons
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h> //提供宏定义 INADDR_ANY
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define MSG_LEN 50
#define IP_LEN 20
#define SERVICE_PROT 2234
#define CLIENT_NUM 1000

/*客户端的结构体*/
typedef struct client_list {
    int c_fd;
    char ip[IP_LEN];

    struct client_list *next, *client_list_head;
} CL, *P_CL;

/*服务器的结构体*/
typedef struct service_inf {
    int s_fd;
    int client_num;
    pthread_t w_id; // 服务器等待客户端和键盘接收的线程
    struct sockaddr_in s_inf;
    pthread_t c_pid[CLIENT_NUM];
    P_CL client_list_head; // 存放客户端链表头
} SI, *P_SI;

P_SI p_si;

P_SI Service_Init();                           // 初始化服务器
void *Wait_For_Client(void *arg);              // 等待客户端连接
void *Service_Broadcast(void *arg);            // 广播消息文件
int Service_Working(P_SI p_si);                // 服务器运行
P_CL Creta_List_Node();                        // 创建客户端链表结点
int Del_Client_Node(P_CL head, P_CL del_node); // 删除客户端节点
int Service_Free(P_SI p_si);                   // 服务器释放/*这个函数可能要改，不然可能会反复执行*/
void tcp_broadcast_init();                     // 初始化tcp
int broadcast_task();                          // 广播任务
int UI_broadcast();