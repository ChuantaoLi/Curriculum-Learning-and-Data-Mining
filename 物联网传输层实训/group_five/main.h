#define _BSD_SOURCE
#define _DEFAULT_SOURCE
#include "LCD.h"
#include "blue.h"
#include "client_file.h"
#include "tcp_broadcast.h"
#include "touch.h"

int flag;

/*LCD变量*/
extern Node *picture;
extern int picture_count;

extern int touch_x, touch_y;
extern char recv_buf[1024];
