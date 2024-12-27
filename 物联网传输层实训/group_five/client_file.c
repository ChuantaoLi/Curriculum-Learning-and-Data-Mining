// 客户端发送文件
#include "main.h"

#define MAX_BUF_SIZE 1024

int sockfd; // 服务器套接字

// 发送数据的线程函数
// 替换发送数据逻辑为文件传输
void *send_data(void *arg) {

    char file_path[MAX_BUF_SIZE];
    FILE *file;
    char buffer[MAX_BUF_SIZE];
    ssize_t bytes_read;

    while (1) {
        // if(flag==4)
        {
            printf("请输入要发送的文件路径：");
            fgets(file_path, sizeof(file_path), stdin);
            file_path[strcspn(file_path, "\n")] = '\0'; // 去掉换行符

            if (strcmp(file_path, "exit") == 0) {
                printf("退出程序\n");
                close(sockfd);      // 关闭套接字
                pthread_exit(NULL); // 退出线程
            }

            file = fopen(file_path, "rb");
            if (!file) {
                perror("打开文件失败");
                continue;
            }

            // 发送文件名称
            char *file_name = strrchr(file_path, '/');
            file_name = file_name ? file_name + 1 : file_path;
            write(sockfd, file_name, strlen(file_name) + 1);

            // 发送文件数据
            while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
                if (write(sockfd, buffer, bytes_read) == -1) {
                    perror("发送文件失败");
                    break;
                }
            }
            fclose(file);
            printf("文件发送完成：%s\n", file_path);
        }
    }

    return NULL;
}

// 修改接收数据逻辑为文件接收
void *receive_data(void *arg) {

    char buffer[MAX_BUF_SIZE];
    char file_name[MAX_BUF_SIZE];
    ssize_t bytes_received;
    FILE *file;

    while (1) {
        // if(flag==4)
        {
            // 接收文件名称
            bytes_received = read(sockfd, file_name, sizeof(file_name) - 1);
            if (bytes_received <= 0) {
                perror("接收文件名称失败");
                close(sockfd);
                pthread_exit(NULL);
            }
            file_name[bytes_received] = '\0';
            printf("接收文件：%s\n", file_name);

            file = fopen(file_name, "wb");
            if (!file) {
                perror("创建文件失败");
                continue;
            }
            addNode(&picture, file_name);
            // 接收文件数据
            while ((bytes_received = read(sockfd, buffer, sizeof(buffer))) > 0) {
                fwrite(buffer, 1, bytes_received, file);
            }
            fclose(file);
            printf("文件接收完成：%s\n", file_name);
        }
    }

    return NULL;
}

void client_file_init() {
    // 第一步：创建套接字
    sockfd = socket(AF_INET, SOCK_STREAM, 0); // 使用 TCP 协议
    if (sockfd == -1) {
        perror("创建套接字失败");
        return ;
    }
    printf("套接字创建成功\n");

    // 第二步：连接服务器
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8888);                      // 服务器端口
    server_addr.sin_addr.s_addr = inet_addr("192.168.1.10"); // 服务器地址（假设是本机）

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("连接服务器失败");
        close(sockfd); // 关闭套接字
        return ;
    }
    printf("连接服务器成功\n");

    // 创建发送和接收线程
    pthread_t send_thread, recv_thread;

    // 创建发送数据线程
    if (pthread_create(&send_thread, NULL, send_data, NULL) != 0) {
        perror("创建发送线程失败");
        close(sockfd); // 关闭套接字
        return ;
    }

    // 创建接收数据线程
    if (pthread_create(&recv_thread, NULL, receive_data, NULL) != 0) {
        perror("创建接收线程失败");
        close(sockfd); // 关闭套接字
        return ;
    }

    // // 等待线程结束
    pthread_join(send_thread, NULL);
    pthread_join(recv_thread, NULL);
}
