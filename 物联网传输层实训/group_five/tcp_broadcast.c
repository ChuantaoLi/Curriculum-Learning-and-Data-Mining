// 多人聊天室
#include "main.h"

extern int touch_x, touch_y;
P_SI Service_Init() // 初始化服务器
{
    // malloc申请堆空间
    P_SI p_si = (P_SI)malloc(sizeof(SI));
    if (p_si == NULL) {
        perror("malloc");
        return (P_SI)-1;
    }

    memset(p_si, 0, sizeof(SI));
    p_si->client_num = 0;

    // 创建套接字
    p_si->s_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (p_si->s_fd == -1) {
        perror("socket");
        return (P_SI)-1;
    }
    printf("%d\n", p_si->s_fd);

    p_si->s_inf.sin_port = htons(SERVICE_PROT);
    p_si->s_inf.sin_family = AF_INET;
    // p_si->s_inf.sin_addr.s_addr = htonl(INADDR_ANY);
    p_si->s_inf.sin_addr.s_addr = inet_addr("192.168.0.18"); // wife的节点
    int on = 1;
    // 取消端口绑定限制
    setsockopt(p_si->s_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    int bind_ret = bind(p_si->s_fd, (struct sockaddr *)&p_si->s_inf, sizeof(p_si->s_inf));
    if (bind_ret == -1) {
        perror("bind");
        return (P_SI)-1;
    }

    int listen_ret = listen(p_si->s_fd, 30);
    if (listen_ret == -1) {
        perror("listen");
        return (P_SI)-1;
    }

    // 创建客户端链表头
    p_si->client_list_head = (P_CL)malloc(sizeof(CL));
    if (p_si->client_list_head == NULL) {
        perror("malloc");
        return (P_SI)-1;
    }
    memset(p_si->client_list_head, 0, sizeof(CL));
    p_si->client_list_head->next = NULL;

    return p_si;
}

int Del_Client_Node(P_CL head, P_CL del_node) // 删除客户端节点
{
    if (head == NULL) {
        printf("非法头节点，删除异常！\n");
        return -1;
    } else if (head->next == NULL) {
        printf("空的客户端链表，删除失败！\n");
        return -1;
    } else {
        for (P_CL befor_node = head; befor_node->next != NULL; befor_node = befor_node->next) {
            if (befor_node->next == del_node) {
                befor_node->next = del_node->next;

                del_node->next = NULL;
                free(del_node);
                break;
            }
        }
    }
    return 0;
}

void *Service_Broadcast(void *arg) // 广播消息文件
{
    P_CL client_node = (P_CL)arg;
    printf("%s连接成功!\n", client_node->ip);
    char msg[MSG_LEN];
    char s_msg[MSG_LEN];
    while (1) {
        memset(msg, 0, MSG_LEN);
        memset(s_msg, 0, MSG_LEN);
        int read_ret = read(client_node->c_fd, msg, MSG_LEN);
        if (read_ret <= 0) {
            printf("%s掉线了\n", client_node->ip);
            // client_node->c_fd = -999;
            Del_Client_Node(client_node->client_list_head, client_node);
            break;
        }
        if (strcmp(msg, "EXIT") == 0) { // 判断客户端是不是想退出 {
            write(client_node->c_fd, msg, strlen(msg));
            printf("%s:ip即将下线！\n", client_node->ip);
            Del_Client_Node(client_node->client_list_head, client_node);
            break;
        }

        sprintf(s_msg, "%s:%s", client_node->ip, msg);

        for (P_CL tmp_node = client_node->client_list_head->next; tmp_node != NULL; tmp_node = tmp_node->next) // for循环广播 {
        {
            if (tmp_node->c_fd == client_node->c_fd)
                continue; // 忽略当事人
            write(tmp_node->c_fd, s_msg, strlen(s_msg));
        }
    }
    pthread_exit(NULL);
}

void *Wait_For_Client(void *arg) // 等待客户端连接
{
    P_SI p_si = (P_SI)arg;
    struct sockaddr_in c_inf;
    socklen_t len = sizeof(c_inf);

    while (1) {
        memset(&c_inf, 0, sizeof(c_inf));
        int c_fd = accept(p_si->s_fd, (struct sockaddr *)&c_inf, &len);
        if (c_fd == -1) {
            perror("accept");
            pthread_exit(NULL);
        }
        /*创建链表结点*/
        P_CL client_node = Creta_List_Node();
        if (client_node == (P_CL)-1) {
            printf("创建客户端结点失败!\n");
            pthread_exit(NULL);
        }
        client_node->c_fd = c_fd;
        strcpy(client_node->ip, inet_ntoa(c_inf.sin_addr));
        client_node->client_list_head = p_si->client_list_head;
        // 头插添加进链表
        client_node->next = p_si->client_list_head->next;
        p_si->client_list_head->next = client_node;
        // 为每一个客户端单独创建一条线程
        int pthread_create_ret = pthread_create(&p_si->c_pid[p_si->client_num], NULL, Service_Broadcast, client_node);
        if (pthread_create_ret != 0) {
            perror("pthread_create");
            pthread_exit(NULL);
        }
        p_si->client_num++;
    }
    pthread_exit(NULL);
}

P_CL Creta_List_Node() // 创建客户端链表结点
{
    P_CL node = (P_CL)malloc(sizeof(CL));
    if (node == NULL) {
        perror("malloc");
        return (P_CL)-1;
    }
    memset(node, 0, sizeof(CL));
    node->next = NULL;
    return node;
}

int Service_Working(P_SI p_si) // 服务器运行
{
    int pthread_create_ret = pthread_create(&p_si->w_id, NULL, Wait_For_Client, (void *)p_si);
    if (pthread_create_ret != 0) {
        perror("pthread_create");
        return -1;
    }
    return 0;
}

int Service_Free(P_SI p_si) // 关闭服务器
{
    // 第一步 群发给客户端让其知道服务器即将退出！
    for (P_CL tmp_node = p_si->client_list_head->next; tmp_node != NULL; tmp_node = tmp_node->next) {
        int write_ret = write(tmp_node->c_fd, "EXIT", 4);
        if (write_ret == -1) {
            perror("write");
            return -1;
        }
    }
    sleep(1);
    // 关闭套接字
    close(p_si->s_fd);

    // 第二步 给所有的线程join回收
    for (int n = 0; n < p_si->client_num; n++) {
        pthread_join(p_si->c_pid[n], NULL);
    }

    // pthread_cancel(p_si->w_id);
    // pthread_join(p_si->w_id,NULL);

    // 第三步删除销毁链表
    free(p_si->client_list_head);

    // 第四步 free这个p_si
    free(p_si);

    return 0;
}

void tcp_broadcast_init() // 初始化tcp
{
    p_si = Service_Init();
    if (p_si == (P_SI)-1) {
        printf("服务器初始化失败！\n");
        return ;
    } else {
        printf("服务器初始化成功！\n");
    }
    // 服务器初始化完之后，需要自己创建一条线程去等待客户端的连接，自己去读取键盘输入即可，如果输入exit 退出程序
    int service_woring_ret = Service_Working(p_si);
    if (service_woring_ret == -1) {
        printf("服务器运行失败！\n");
        return ;
    }
    UI_broadcast(); // 初始化ui界面
}

int broadcast_task() // 广播任务
{

    int service_woring_ret = Service_Working(p_si);
    if (service_woring_ret == -1) {
        printf("服务器运行失败！\n");
        return -1;
    }
    char mask[5];

    int num_2 = 0;

    while (1) {
        if (touch_x > 700 && touch_x < 800 && touch_y > 0 && touch_y < 100) // 退出
        {
            num_2++;
            if (num_2 > 100) // 延时一下
            {
                num_2 = 0;
                break;
            }
        }
    }

    return 0;
}

int UI_broadcast() {
    display_picture(UI_picture, 1);
}
