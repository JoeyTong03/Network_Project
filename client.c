#include "common.h"

#define BUFSIZE 100000
#define STUNO	"1551445"
void communicate(int conn,int pid,int isfork,int socket_id);
int main(int argc, char *argv[])
{
	struct SocketPara socketPara;
	GetSocketPara(argc, argv, &socketPara, 0);//对命令行参数进行处理，完成赋值

	/*创建客户端套接字*/
	int client_sockfd;
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

	if (socketPara.isFork == 1)//以fork方式产生连接
	{
		int i;
		pid_t pid;
		for (i = 0; i < socketPara.connectNum;)
		{
			pid = fork();
			if (pid == -1)//fork子进程失败
			{
				perror("[Fork error]\n");
				return -1;
			}
			else if (pid == 0)//子进程
			{
				i++;
				int conn;
				if (socketPara.isBlock == 1)//阻塞方式
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
				else//非阻塞方式
				{
					fd_set fds;
					FD_ZERO(&fds);
					FD_SET(client_sockfd, &fds);
					switch (select(client_sockfd + 1, &fds, NULL, NULL, NULL))
					{
					case -1:
						perror("select error");
						break;
					case 0:
						sleep(1);
						printf("timeout\n");
						break;
					default:
						conn=connect(client_sockfd, (struct sockaddr*)&remote_addr, sizeof(remote_addr));
					}
					communicate(conn, getpid(), 1, i);
				}
			}
			else
				continue;
		}
	}
	else //由主进程产生num个connect
	{

	}
	close(client_sockfd);
	return 0;
}

void setFilepath(int i, char* filepath)
{
	char str[10];
	itoa(i, str,10);
	strcpy(filepath, str);
	strcat(filepath, ".dat");
}
void getLocalTime(char s[])
{
	time_t ttime;
	ttime = time(NULL);
	struct tm *stTime = localtime(&ttime);
	// 以年月日的形式输出时间
	sprintf(s,"%d-%02d-%02d %02d:%02d:%02d\n",
		stTime->tm_year + 1900,
		stTime->tm_mon + 1,
		stTime->tm_mday,
		stTime->tm_hour,
		stTime->tm_min,
		stTime->tm_sec);
}
void getFileName(int pid, char fileName[])
{
	memset(fileName, 0, 64);
	char pid_str[10];
	itoa(pid, pid_str, 10);
	strcpy(fileName, STUNO);
	strcat(fileName, ".");
	strcat(fileName, pid_str);
	strcat(fileName, ".pid.txt");
}
void writeToFile(int pid_tmp, char timestr[], char randData_bak, char fileName[])
{
	FILE *fp = fopen(fileName, "w");
	if (fp == NULL)
	{
		perror("File open fail!\n");
		exit(-1);
	}
	fputs(STUNO, fp);
	fputc('\n\t', fp);
	char pid_str[10];
	itoa(pid_tmp, pid_str, 10);
	fputs(pid_str, fp);
	fputc('\n\t', fp);
	fputs(timestr, fp);
	fputc('\n\t', fp);
	fputs(randData_bak, fp);
}

//连接成功后与server端的通信过程，包括将四项信息写入学号.进程号.pid.txt
void communicate(int conn,int pid,int isfork,int socket_id)
{
	char readBuf[BUFSIZE];
	char writeBuf[BUFSIZE];
	memset(writeBuf, 0, BUFSIZE);
	memset(readBuf, 0, BUFSIZE);
	if (read(conn, readBuf, 5) < 0)//接收S发送的StuNo,不含尾零
	{
		perror("[read-1]");
		exit(-1);
	}
	readBuf[5] = '\0';
	if (strcmp(readBuf, "StuNo") != 0)//接收到的信息不等于StuNo
	{
		close(conn);//中断连接
		printf("Connection_%d closed!", pid);
		exit(0);
	}

	int stuno = htonl(STUNO);
	if (write(conn, &stuno, 4) < 0)//发送网络序的学号,int
	{
		perror("[write-1]");
		exit(-1);
	}

	/*---------------------------------------------------------------------*/
	memset(readBuf, 0, BUFSIZE);
	if (read(conn, readBuf, 3) < 0)//接收S发送的pid，不含尾零
	{
		perror("[read-2]");
		exit(-1);
	}
	readBuf[3] = '\0';
	if (strcmp(readBuf, "pid") != 0)//接收到的信息不等于"pid"
	{
		close(conn);//中断连接
		printf("Connection_%d closed!", pid);
		exit(0);
	}

	int pid_tmp;
	if (isfork == 1)
		pid_tmp = htonl(pid);
	else
		pid_tmp = htonl(pid << 16 + socket_id);

	if (write(conn, &pid_tmp, 4) < 0)
	{
		perror("[write-2]");
		exit(-1);
	}
	/*=-------------------------------------------------------------*/
	char timestr[30];
	memset(timestr, 0, 30);
	memset(readBuf, 0, BUFSIZE);
	if (read(conn, readBuf, 5) < 0)//接收S发送的TIME，含尾零
	{
		perror("[read-3]");
		exit(-1);
	}
	if (strcmp(readBuf, "TIME") != 0)//接收到的信息不等于"TIME"
	{
		close(conn);//中断连接
		printf("Connection_%d closed!", pid);
		exit(0);
	}
	memset(writeBuf, 0, BUFSIZE);
	getLocalTime(timestr);
	if (write(conn, timestr, 19) < 0)
	{
		perror("[write-3]");
		exit(-1);
	}
	/*-----------------------------------------------------------------*/
	memset(readBuf, 0, BUFSIZE);
	if (read(conn, readBuf, 9) < 0)//接收S发送的"str*****"，含尾零
	{
		perror("[read-4]");
		exit(-1);
	}
	char tmp[10];
	strncpy(tmp, readBuf+3, 5);
	int randNumber = atoi(tmp);
	if (strncmp(readBuf, "str",3) != 0 | randNumber<32768)//接收到的信息不等于"str*****"
	{
		close(conn);//中断连接
		printf("Connection_%d closed!", pid);
		exit(0);
	}
	
	char randData_bak[BUFSIZE]; //备份，用于后续写入文件
	for (int i = 0; i < randNumber; i++)
	{
		writeBuf[i] = rand() % 256;
		randData_bak[i] = writeBuf[i];
	}
	writeBuf[randNumber] = '\0';
	randData_bak[randNumber] = '\0';

	int nwritten, total = 0;
	while (total != randNumber) 
	{
		nwritten = write(conn, writeBuf, randNumber - total);
		if (nwritten == 0)  
			break; /*这里如果设置了非阻塞写入的话，有可能是sendbuffer满了*/
		if (nwritten == -1) 
			exit(-1); /*写入出错，可能对方关关闭了连接*/
		total += nwritten;
		strncpy(writeBuf, writeBuf + nwritten, randNumber - total);
		writeBuf[randNumber - total] = '\0';
	}
	/*-------------------------------------------------------------------------*/
	memset(readBuf, 0, BUFSIZE);
	if (read(conn, readBuf, 3) < 0)//接收S发送的end，不含尾零
	{
		perror("[read-5]");
		exit(-1);
	}
	readBuf[3] = '\0';
	if (strcmp(readBuf, "end") != 0)//接收到的信息不等于"end"
	{
		close(conn);//中断连接
		printf("Connection_%d closed!", pid);
		exit(0);
	}
	//将四项信息写入文件
	char fileName[64];
	getFileName(pid, fileName);
	writeToFile(pid_tmp, timestr, randData_bak,fileName);
}