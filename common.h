

#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>

extern const int TRUE;
extern const int FALSE;
extern char *SERVER_DEFAULTIP;

struct SocketPara
{
    char *ip;       //ip
    int port;       //端口号
    int connectNum; //连接数
    int isBlock;    //是否为阻塞方式
    int isFork;     //是否为分裂进程方式
};
