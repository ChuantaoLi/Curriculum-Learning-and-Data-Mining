// 图片显示
#include "LCD.h"
#define WIDTH 800
#define HEIGH 480
int *plcd = NULL;
extern int touch_x;
extern int touch_y;
int picture_num, num_1 = 0;
// 初始化链表
Node *initList() {
    Node *head = (Node *)malloc(sizeof(Node));
    if (head == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(1);
    }
    head->next = NULL;
    return head;
}

void addNode(Node **head, const char *filename) // 添加一个文件
{
    Node *newNode = (Node *)malloc(sizeof(Node));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(1);
    }
    strcpy(newNode->filename, filename);
    newNode->next = (*head)->next;
    (*head)->next = newNode;
}

void deleteNode(Node **head, const char *filename) // 删除一个文件
{
    Node *current = *head;
    Node *prev = NULL;
    while (current->next != NULL) {
        if (strcmp(current->next->filename, filename) == 0) {
            Node *temp = current->next;
            current->next = temp->next;
            free(temp);
            return;
        }
        prev = current;
        current = current->next;
    }
    printf("Filename %s not found.\n", filename);
}

int findNode(Node *head, const char *filename) // 查找文件/输入文件名字查找//找到返回1
{
    Node *current = head->next;
    while (current != NULL) {
        if (strcmp(current->filename, filename) == 0) {
            return 1; // Found
        }
        current = current->next;
    }
    return 0; // Not found
}

char *getNodeAtIndex(Node *head, int index) // 查找第index个文件，索引第一个是0
{
    if (head == NULL || index < 0) {
        return NULL;
    }

    Node *current = head;  // 从头节点开始
    int currentIndex = -1; // 头节点不计数

    while (current != NULL && currentIndex < index) {
        current = current->next;
        currentIndex++;
    }

    if (currentIndex != index) {
        return NULL; // 索引超出范围
    }

    return current->filename; // 返回第 index 个节点的文件名
}

int printList(Node *head) // 打印所有图片名字
{
    Node *current = head->next;
    int count = 0;
    while (current != NULL) {
        count++;
        printf("%s\n", current->filename);
        current = current->next;
    }
    return count;
}

void LCD_init() {
    picture = initList();
    UI_picture = initList();
    addNode(&UI_picture, "/images/wifi.bmp"); // 聊天室背景图
    addNode(&UI_picture, "/images/menu.bmp"); // 桌面背景图

    addNode(&picture, "/images/1.bmp");
    addNode(&picture, "/images/2.bmp");
    addNode(&picture, "/images/3.bmp");
    addNode(&picture, "/images/4.bmp");
    addNode(&picture, "/images/5.bmp");

    picture_count = printList(picture);
    ui_picture_count = printList(UI_picture);
}

void rotate(int src[], int dest[], int width, int height) {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            dest[i + width * j] = src[i + width * (height - j - 1)];
        }
    }
}

int display_picture(Node *head, int i) {
    char *buff = getNodeAtIndex(head, i); // 查找第index个文件，索引第一个是0
    int fb0 = open("/dev/fb0", O_RDWR);
    int bmp = open(buff, O_RDWR);
    if (fb0 == -1 || bmp == -1) {
        // printf("open lcd failed\n");
        return -1;
    } else {
        // printf("open lcd success\n");
    }

    // mmap the framebuffer device
    plcd = (int *)mmap(NULL, WIDTH * HEIGH * 4, PROT_READ | PROT_WRITE, MAP_SHARED, fb0, 0);
    if (plcd == MAP_FAILED) {
        // printf("mmap failed\n");
        close(fb0);
        close(bmp);
        return -1;
    }

    char buf[WIDTH * HEIGH * 3];
    lseek(bmp, 54, SEEK_SET);
    int rd = read(bmp, buf, WIDTH * HEIGH * 3);
    if (rd == -1) {
        // printf("read bmp failed\n");
    } else {
        // printf("read bmp success\n");
    }

    // Convert BMP data to ARGB
    int argb[WIDTH * HEIGH] = {0};
    for (int i = 0; i < WIDTH * HEIGH; i++) {
        argb[i] = (buf[3 * i + 2] << 16) | (buf[3 * i + 1] << 8) | buf[3 * i];
    }

    // Rotate the image
    int argb_rotated[WIDTH * HEIGH] = {0};
    rotate(argb, argb_rotated, WIDTH, HEIGH);

    // Copy rotated image to framebuffer
    for (int i = 0; i < WIDTH * HEIGH; i++) {
        plcd[i] = argb_rotated[i];
    }

    // Unmap and close files
    munmap(plcd, WIDTH * HEIGH * 4);
    close(fb0);
    close(bmp);

    return 0;
}

int LCD_task() // 图片显示任务
{

    while (1) {
        if (touch_x > 600 && touch_x < 800 && touch_y > 280 && touch_y < 480) // 下一张图片
        {
            num_1++;
            if (num_1 > 50) // 延时一下
            {
                if (picture_num >= picture_count)
                    picture_num = 1;
                display_picture(picture, picture_num);
                picture_num++;
                num_1 = 0;
            }
        } else {
            if (touch_x > 0 && touch_x < 200 && touch_y > 280 && touch_y < 480) // 上一张图片
            {
                num_1++;
                if (num_1 > 50) // 延时一下
                {
                    if (picture_num >= picture_count)
                        picture_num = 1;
                    display_picture(picture, picture_num);
                    picture_num--;
                    num_1 = 0;
                }
            } else {
                if (touch_x > 700 && touch_x < 800 && touch_y > 0 && touch_y < 100) // 推出
                {
                    num_1++;
                    if (num_1 > 50) // 延时一下
                    {

                        picture_num = 0;
                        num_1 = 0;
                        break;
                    }
                }
            }
        }
        if (picture_num >= picture_count)
            picture_num = 1;
        display_picture(picture, picture_num);
    }
    return 0;
}

void LCD_desktop() {

    display_picture(UI_picture, 0);
}
