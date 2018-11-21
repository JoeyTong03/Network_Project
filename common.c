
#include "common.h"
const int TRUE = 1;
const int FALSE = 0;
char *SERVER_DEFAULTIP = "0.0.0.0";

// struct SocketPara
// {
//     char *ip;       //ip
//     int port;       //端口号
//     int connectNum; //连接数
//     int isBlock;    //是否为阻塞方式
//     int isFork;     //是否为分裂进程方式
// };

/**********************
 * 函数名称：strcasecmp                             
 * 功    能：判断两个字符串是不是相等，不涉及大小写                  
 * 参    数：const char* s1,const char* s2 - 两个字符串
 * 返    回：int - 返回两个参数是不是相等 TRUE/FALSE         
 * 说    明：不区分大小写                               
***********************/
int strcasecmp(const char *s1, const char *s2)
{
    int c1 = 0, c2 = 0;
    do
    {
        c1 = tolower(*s1++);
        c2 = tolower(*s2++);
    } while (c1 == c2 && c1 != 0);

    return (c1 == c2) ? TRUE : FALSE;
}

/**********************
 * 函数名称：IsIPAvailable                       
 * 功    能：判断IP是否符合标准
 * 参    数：const char* const ip
 * 返    回：TRUE/FALSE - ip符合规范/不符合规范
 * 说    明：
***********************/
int IsIPAvailable(char *ip)
{
    int i = 0, num = 0;

    //ip全部为数字与'.'
    for (i = 0; ip[i] != 0; i++)
        if (ip[i] < '0' || ip[i] > '9')
        {
            if (ip[i] == '.')
                num++;
            else
                return FALSE;
        }

    if (num != 3)
        return FALSE;

    //每一段要求符合0-255
    char tmpIP[20] = {0};
    strcpy(tmpIP, ip);
    char *tmp = strtok(tmpIP, ".");
    for (i = 0; i < 4; i++)
    {
        //防止 192.17.2. 这种情况
        if (tmp == NULL)
            return FALSE;

        int tmpNum = atoi(tmp);
        if (tmpNum < 0 || tmpNum > 255)
            return FALSE;

        tmp = strtok(NULL, ".");
    }

    return TRUE;
}

/**********************
 * 函数名称：GetSocketPara                  
 * 功    能：客户端、服务端的命令行参数解析
 * 参    数：int argc, char *argv[]     - 命令行参数
 *           struct SocketPara *obj,   - socket参数
 *           int isServer=TRUE         - 判断当前是否为客户端
 * 返    回：
 * 说    明：
***********************/
int GetSocketPara(int argc, char *argv[], struct SocketPara *obj, const int isServer)
{
    /*初始化*/
    {
        obj->ip = NULL;
        obj->connectNum = 100;
        obj->isFork = FALSE;
        obj->isBlock = FALSE;
        obj->port = -1;
    }

    /*命令行参数解析*/
    int i = 1;
    int isNonBlockExist = FALSE;
    for (; i < argc; i++)
    {
        //ip
        if (strcasecmp(argv[i], "--ip") == TRUE)
        {
            i++;
            if (i == argc)
            {
                printf("IP error\n");
                exit(-1);
            }

            //判断ip是否符合规范
            if (IsIPAvailable(argv[i]) == FALSE)
            {
                printf("IP error\n");
                exit(-1);
            };

            obj->ip = argv[i];
        }

        //port
        if (strcasecmp(argv[i], "--port") == TRUE)
        {
            i++;
            if (i == argc)
            {
                printf("port error\n");
                exit(EXIT_FAILURE);
            }

            //判断port是否符合规范
            int tmpport = atoi(argv[i]);
            if (tmpport < 0)
            {
                printf("port error\n");
                exit(EXIT_FAILURE);
            }

            obj->port = tmpport;
        }

        //block
        if (strcasecmp(argv[i], "--block") == TRUE)
        {
            //如果block 与 nonblock同时出现,block无效
            if (isNonBlockExist == FALSE)
                obj->isBlock = TRUE;
        }

        //nonblock
        if (strcasecmp(argv[i], "--nonblock") == TRUE)
        {
            //如果block 与 nonblock同时出现,block无效
            obj->isBlock = FALSE;
            isNonBlockExist = TRUE;
        }

        //fork
        if (strcasecmp(argv[i], "--fork") == TRUE)
        {
            obj->isFork = TRUE;
        }

        //nofork
        if (strcasecmp(argv[i], "--nofork") == TRUE)
        {
            obj->isFork = FALSE;
        }

        //num
        if (strcasecmp(argv[i], "--num") == TRUE)
        {
            i++;
            if (i == argc)
            {
                printf("num error\n");
                exit(EXIT_FAILURE);
            }

            //判断port是否符合规范
            int tmpnum = atoi(argv[i]);
            if (tmpnum < 0)
            {
                printf("num error\n");
                exit(EXIT_FAILURE);
            }

            obj->connectNum = tmpnum;
        }
    }

    /*缺省添加*/
    if (obj->ip == NULL)
    {
        if (isServer == TRUE)
            obj->ip = SERVER_DEFAULTIP;
        else
        {
            printf("lack client ip\n");
            exit(EXIT_FAILURE);
        }
    }

    if (obj->port == -1)
    {
        printf("lack port\n");
        exit(EXIT_FAILURE);
    }
}

