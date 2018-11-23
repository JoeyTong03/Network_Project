#include "client_func.h"
#define MAXCONN 1200
int main(int argc, char *argv[])
{
	struct SocketPara socketPara;
	GetSocketPara(argc, argv, &socketPara, 0);//对命令行参数进行处理，完成赋值
	if (socketPara.isFork == 1)//以fork的形式创建连接
	{
		int i = 0;
		pid_t pid = 0;
		for (i = 0; i<socketPara.connectNum; i++)
		{
			pid = fork();
			if (pid == -1)//fork子进程失败
			{
				perror("[Fork error]\n");
				return -1;
			}
			else if (pid == 0)//子进程
			{
				int client_sockfd;
				int conn;
				if ((client_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
				{
					perror("[Create socket error]\n");
					return -1;
				}
				//设置非阻塞方式
				if (socketPara.isBlock == 0)
					SetSocketNonblock(client_sockfd);

				//服务器IP，端口号绑定
				struct sockaddr_in remote_addr;//客户端网络地址结构体
				memset(&remote_addr, 0, sizeof(remote_addr));
				remote_addr.sin_family = AF_INET;//通信协议为因特网
				remote_addr.sin_port = htons(socketPara.port);//服务器端口号
				remote_addr.sin_addr.s_addr = inet_addr(socketPara.ip);//服务器IP地址

				if (socketPara.isBlock == 0)//非阻塞方式
				{

				}
				else //阻塞方式
				{
					if ((conn = connect(client_sockfd, (struct sockaddr*)&remote_addr, sizeof(remote_addr))) < 0)
					{
						perror("connect error");
						return -1;
					}
					printf("[connect-%d success]\n", i);
					communicate(conn, getpid(), 1, i);
					exit(0);
				}	
			}
			else
			{
				while (1)
					sleep(2);
			}
		}
	}
	else //由主进程完成创建，异步
	{
		int client_sockfd[MAXCONN], conn[MAXCONN];

	}
}