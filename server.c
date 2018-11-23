
//2018.11.22 杨小宇

#include "common.h"

//连接的最大数
//因为不让使用poll和epoll，fd_set最大只能有1024个连接，所以不能超过1024
#define MAXCONNECTION 10

/*************************************
 * 函数名称：startup
 * 功   能：开始socket连接
 * 参   数：int _port      - 端口名称 const char* _ip - IP地址
 * 返   回：套接口
 * 说   明：
*************************************/
int startup(int _port, const char *_ip, const int isBlock)
{
    /* 创建套接字 */
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("socket error");
        exit(1);
    }

    //如果要求为非阻塞则进行非阻塞设置
    if (isBlock == FALSE)
    {
        int val = 0;
        if ((val = fcntl(sock, F_GETFL, 0)) < 0)
        {
            close(sock);
            exit(-2);
        }
        if (fcntl(sock, F_SETFL, val | O_NONBLOCK) < 0)
        {
            close(sock);
            exit(-3);
        }
    }

    /* 服务器绑定一个固定的网络地址和端口号 */
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(_port); //主机字节序转换为网络字节序
    server_addr.sin_addr.s_addr = inet_addr(_ip);
    socklen_t len = sizeof(server_addr);

    // int bind(int sockfd,const struct sockaddr*addr,socklen_t addrlen);
    // sockfd：服务器打开的sock
    if (bind(sock, (struct sockaddr *)&server_addr, len) < 0)
    {
        perror("bind error");
        exit(2);
    }

    /* 监听 */

    if (listen(sock, 5) < 0)
    {
        perror("listen error");
        exit(3);
    }

    return sock;
}

/*************************************
 * 函数名称：SeverSendHeadInfo
 * 功   能：非阻塞形式的服务
 * 参   数：
 * 返   回：
 * 说   明：
*************************************/
void ServerSendHeadInfo(int* _step,int _serversocket)
{
    int step=*_step;
    int ServerSocket=_serversocket;
    char head[5][10]={
        "StuNo",    //不发尾零
        "pid",      //不发尾零
        "TIME",     //发送尾零
        "str54321", //发送尾零
        "end"       //不发尾零
    };

    if(step==0&&step==1&&step==4)
    {
        /* 不发送尾零 */
        write(ServerSocket,head[step],strlen(head[step])-1);
    }
    else if(step<5)
    {
        /* 发送尾零 */
        write(ServerSocket,head[step],strlen(head[step]));
    }

    _step++;
}

/*************************************
 * 函数名称：nonBlockServer
 * 功   能：非阻塞形式的服务
 * 参   数：
 * 返   回：
 * 说   明：
*************************************/
void nonBlockServer(const int _serversock)
{
    /* 服务端socket连接 */
    int ServerSocket = _serversock;

    /* 客户端socket连接 */
    int ClientSocket[MAXCONNECTION] = {0};

    /* 创建文件描述符 */
    fd_set rfd; //读
    fd_set wfd; //写

    /* 超时时间设置 */
    struct timeval tv;

    /* select监视的文件描述符的个数 */
    int Maxfd = ServerSocket;

    while (1)
    {
        /* 无阻塞获得所有文件的描述符 */
        tv.tv_sec = 0;
        tv.tv_usec = 0;

        /* 将读写标志位清空为0 */
        FD_ZERO(&rfd);
        FD_ZERO(&wfd);

        /* 将相应的文件进行标志位登记 */
        FD_SET(ServerSocket, &rfd); //读标志位
        FD_SET(ServerSocket, &wfd); //写标志位

        /* 找出当前select当前监视的文件描述符的个数 */
        int i = 0;
        for (i = 0; i < MAXCONNECTION; i++)
        {
            if (ClientSocket[i] != 0)
            {
                FD_SET(ClientSocket[i], &rfd);
                FD_SET(ClientSocket[i], &wfd);

                if (Maxfd < ClientSocket[i])
                {
                    Maxfd = ClientSocket[i];
                }
            }
        }

       /*******
        * 哪个连接当前接受Client数据 - 初始化为0
        * 当前服务端所处的步骤
        ******/
        enum BOOL ReadyForClient[MAXCONNECTION]={FALSE};
        int StepofServer[MAXCONNECTION]={0};

        /* 选择查看那个文件有了变化 */
        int ret = select(Maxfd + 1, &rfd, &wfd, NULL, &tv);
        if (ret < 0)
        {
            perror("select error\n");
            continue;
        }
        else if (ret == 0)
        {
            perror("time out\n");
            continue;
        }
        else
        {
            /* 检查服务端是否发生变化 - 有新连接介入 */
            if (FD_ISSET(ServerSocket, &rfd))
            {
                //有新的连接请求
                struct sockaddr_in ClientAddr;
                socklen_t CAddr_len;
                int newClientSocet = accept(ServerSocket, (struct sockaddr *)&ClientAddr, &CAddr_len);
                printf("new connection:%d\n", newClientSocet);

                if (newClientSocet > 0)
                {
                    int index = -1;
                    for (i = 0; i < MAXCONNECTION; i++)
                    {
                        if (ClientSocket[i] == 0)
                        {
                            index = i;
                            ClientSocket[i] = newClientSocet;
                            break;
                        }
                    }

                    if (index >= 0)
                    {
                        printf("new client %d join successfully\n", index);
                    }
                    else
                    {
                        printf("new client %d's joinning failed\n", index);
                    }
                }
            }

            /* 每一个连接进行遍历 - 查找有变化的文件 */
            for (i = 0; i <= MAXCONNECTION; i++)
            {
                //这个连接存在
                if (ClientSocket[i] != 0)
                {
                    //这个连接发生了变化
                    if (FD_ISSET(ClientSocket[i], &rfd))
                    {
                        /*对客户端进行读的操作*/
                        char buf[1024];
                        int rres=read(ClientSocket[i],buf,1024);
                        if(rres==0)
                        {
                            //文件传输结束或客户端关闭
                            FD_CLR(ClientSocket[i],&rfd);
                            FD_CLR(ClientSocket[i],&wfd);
                            ClientSocket[i]=0;
                            continue;
                        }
                        else if(rres<0)
                        {
                            //文件传输出错
                            perror("服务端文件接受出错\n");
                            continue;
                        }
                        else
                        {
                            buf[rres+1]='\0';
                            printf("服务端接受信息:%s",buf);
                        }
                    }

                    if(FD_ISSET(ClientSocket[i],&wfd))
                    {
                        /*对服务端进行写的操作*/
                        ServerSendHeadInfo(&(StepofServer[i]),ServerSocket);
                    }
                }
            }
        }
    }

    close(ServerSocket);
}

void BlockServer(const int _serversock)
{
    /* 服务端socket连接 */
    int ServerSocket = _serversock;
    int ConnectNum=0;
    for(;connectNum<=MAXCONNECTION;)
    {
        //用来接收客户端的socket地址结构体
        struct sockaddr_in remote;
        socklen_t len = sizeof(struct sockaddr_in);

        int SockNew=accept(ServerSocket,(struct sockaddr*)&remote, &len);
        if(SockNew<0)
        {
            perror("Accept Error!\n");
            continue;
        }
        pid_t pid=fork();
        if(pid > 0)//父进程
            continue;
        else if(connectNum<=1024)
        {
            for(;pid<0;)//若子进程已达到极限数量
            {
                sleep(1);
                pid=fork();
            }
            printf("new connection:%d\n", SockNew);
            connectNum++;
            int i;
            char buf[1024];
            for(i=0;i<6;i++)
            {
                ServerSendHeadInfo(i,SockNew);
                ssize_t _s = read(SockNew, buf, sizeof(buf)-1);
                if(i<5&&_s>0)
                {
                    buf[_s]='\0';
                    printf("服务端接受信息:%s",buf);
                }
                else if(_s<0)
                {
                    perror("服务端文件接受出错\n");
                    continue;
                }
                else
                {
                    //文件传输结束或客户端关闭
                    exit(0);
                }
            }
        }
    }
}

int main(int argc, char const *argv[])
{
    struct SocketPara serverPara;
    
    GetSocketPara(argc, argv, &serverPara, TRUE);

    int ServerSocket=startup(serverPara.port,serverPara.ip,serverPara.isBlock);

    if(serverPara.isBlock==FALSE)
        nonBlockServer(ServerSocket);
    else
        BlockServer(ServerSocket);

    return 0;
}
