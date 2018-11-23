
//2018.11.22 ��С��

#include "common.h"

//���ӵ������
//��Ϊ����ʹ��poll��epoll��fd_set���ֻ����1024�����ӣ����Բ��ܳ���1024
#define MAXCONNECTION 10

/*************************************
 * �������ƣ�startup
 * ��   �ܣ���ʼsocket����
 * ��   ����int _port      - �˿����� const char* _ip - IP��ַ
 * ��   �أ��׽ӿ�
 * ˵   ����
*************************************/
int startup(int _port, const char *_ip, const int isBlock)
{
    /* �����׽��� */
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("socket error");
        exit(1);
    }

    //���Ҫ��Ϊ����������з���������
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

    /* ��������һ���̶��������ַ�Ͷ˿ں� */
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(_port); //�����ֽ���ת��Ϊ�����ֽ���
    server_addr.sin_addr.s_addr = inet_addr(_ip);
    socklen_t len = sizeof(server_addr);

    // int bind(int sockfd,const struct sockaddr*addr,socklen_t addrlen);
    // sockfd���������򿪵�sock
    if (bind(sock, (struct sockaddr *)&server_addr, len) < 0)
    {
        perror("bind error");
        exit(2);
    }

    /* ���� */

    if (listen(sock, 5) < 0)
    {
        perror("listen error");
        exit(3);
    }

    return sock;
}

/*************************************
 * �������ƣ�SeverSendHeadInfo
 * ��   �ܣ���������ʽ�ķ���
 * ��   ����
 * ��   �أ�
 * ˵   ����
*************************************/
void ServerSendHeadInfo(int* _step,int _serversocket)
{
    int step=*_step;
    int ServerSocket=_serversocket;
    char head[5][10]={
        "StuNo",    //����β��
        "pid",      //����β��
        "TIME",     //����β��
        "str54321", //����β��
        "end"       //����β��
    };

    if(step==0&&step==1&&step==4)
    {
        /* ������β�� */
        write(ServerSocket,head[step],strlen(head[step])-1);
    }
    else if(step<5)
    {
        /* ����β�� */
        write(ServerSocket,head[step],strlen(head[step]));
    }

    (*_step)++;
}

/*************************************
 * �������ƣ�nonBlockServer
 * ��   �ܣ���������ʽ�ķ���
 * ��   ����
 * ��   �أ�
 * ˵   ����
*************************************/
void nonBlockServer(const int _serversock)
{
    /* �����socket���� */
    int ServerSocket = _serversock;

    /* �ͻ���socket���� */
    int ClientSocket[MAXCONNECTION] = {0};

    /* �����ļ������� */
    fd_set rfd; //��
    fd_set wfd; //д

    /* ��ʱʱ������ */
    struct timeval tv;

    /* select���ӵ��ļ��������ĸ��� */
    int Maxfd = ServerSocket;

    while (1)
    {
        /* ��������������ļ��������� */
        tv.tv_sec = 0;
        tv.tv_usec = 0;

        /* ����д��־λ���Ϊ0 */
        FD_ZERO(&rfd);
        FD_ZERO(&wfd);

        /* ����Ӧ���ļ����б�־λ�Ǽ� */
        FD_SET(ServerSocket, &rfd); //����־λ
        FD_SET(ServerSocket, &wfd); //д��־λ

        /* �ҳ���ǰselect��ǰ���ӵ��ļ��������ĸ��� */
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
        * �ĸ����ӵ�ǰ����Client���� - ��ʼ��Ϊ0
        * ��ǰ����������Ĳ���
        ******/
        enum BOOL ReadyForClient[MAXCONNECTION]={FALSE};
        int StepofServer[MAXCONNECTION]={0};

        /* ѡ��鿴�Ǹ��ļ����˱仯 */
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
            /* ��������Ƿ����仯 - �������ӽ��� */
            if (FD_ISSET(ServerSocket, &rfd))
            {
                //���µ���������
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

            /* ÿһ�����ӽ��б��� - �����б仯���ļ� */
            for (i = 0; i <= MAXCONNECTION; i++)
            {
                //������Ӵ���
                if (ClientSocket[i] != 0)
                {
                    //������ӷ����˱仯
                    if (FD_ISSET(ClientSocket[i], &rfd))
                    {
                        /*�Կͻ��˽��ж��Ĳ���*/
                        char buf[1024];
                        int rres=read(ClientSocket[i],buf,1024);
                        if(rres==0)
                        {
                            //�ļ����������ͻ��˹ر�
                            FD_CLR(ClientSocket[i],&rfd);
                            FD_CLR(ClientSocket[i],&wfd);
                            ClientSocket[i]=0;
                            continue;
                        }
                        else if(rres<0)
                        {
                            //�ļ��������
                            perror("������ļ����ܳ���\n");
                            continue;
                        }
                        else
                        {
                            buf[rres+1]='\0';
                            printf("����˽�����Ϣ:%s",buf);
                        }
                    }

                    if(FD_ISSET(ClientSocket[i],&wfd))
                    {
                        /*�Է���˽���д�Ĳ���*/
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
    /* �����socket���� */
    int ServerSocket = _serversock;
    int ConnectNum=0;
    for(;ConnectNum<=MAXCONNECTION;)
    {
        //�������տͻ��˵�socket��ַ�ṹ��
        struct sockaddr_in remote;
        socklen_t len = sizeof(struct sockaddr_in);

        int SockNew=accept(ServerSocket,(struct sockaddr*)&remote, &len);
        if(SockNew<0)
        {
            perror("Accept Error!\n");
            continue;
        }
        pid_t pid=fork();
        if(pid > 0)//������
            continue;
        else if(connectNum<=1024)
        {
            for(;pid<0;)//���ӽ����Ѵﵽ��������
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
                ServerSendHeadInfo(&i,SockNew);
                ssize_t _s = read(SockNew, buf, sizeof(buf)-1);
                if(i<5&&_s>0)
                {
                    buf[_s]='\0';
                    printf("����˽�����Ϣ:%s",buf);
                }
                else if(_s<0)
                {
                    perror("������ļ����ܳ���\n");
                    continue;
                }
                else
                {
                    //�ļ����������ͻ��˹ر�
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
