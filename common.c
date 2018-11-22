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
	char tmpIP[20] = { 0 };
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
			if (obj->isBlock == TRUE)//一个主进程处理多个连接必须是非阻塞方式
			{
				printf("error:--nofork is conflict with --block!\n");
				exit(EXIT_FAILURE);
			}

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

/**********************
* 函数名称：mywrite
* 功    能：对write函数系统调用重新封装
* 参    数：fd--文件描述符，buffer--准备写的数据，count--准备写多少数据
filepath--将写的数据备份到本地文件中，默认不备份
* 返    回：int - 返回实际写入的字节数，最大为count
* 说    明：发送缓存有大小限制，故设置循环写入。
***********************/
int MyWrite(int fd, char *buffer, int count, char *filepath)
{
	int nwritten, total = 0;
	FILE *fp;
	if (filepath != NULL)//需要将数据备份到本地文件中
		if ((fp = fopen(filepath, "a+") == NULL))//追加方式打开可读写文件filepath
		{
			perror(filepath);
			return -1;
		}
	while (total != count) {

		nwritten = write(fd, buffer, count - total);
		if (nwritten == 0)
		{
			printf("[Write]:%d Bytes\n", total);
			printf("[Warning]:haven't write entirely!Maybe for non-block settings or sendbuffer is full!\n");
			return total; /*这里如果设置了非阻塞写入的话，有可能是sendbuffer满了*/
		}
		if (nwritten == -1)
		{
			printf("[Error]:write error!Maybe for the other side has close the connection\n");
			return -1; /*写入出错，可能对方关关闭了连接*/
		}
		total += nwritten;
		buffer += nwritten;
		if (filepath != NULL)//需要将数据备份到本地文件中
			if ((fputs(buffer, fp) == NULL))
			{
				perror("[file write error]");
				return -1;
			}
	}
	printf("[Write]:%d Bytes", total);
	if (filepath != NULL)
		fclose(fp);
	return total; /*返回已经写入的字节数*/
}

/**********************
* 函数名称：myread
* 功    能：对read函数系统调用重新封装
* 参    数：fd--文件描述符，buffer--存放读入的数据，count--准备读多少数据
filepath--将读取的数据备份到本地文件中，默认不备份
* 返    回：int - 返回实际读取的字节数，最大为count
* 说    明：接收缓存有大小限制，故设置循环读取。
***********************/
int MyRead(int fd, char *buffer, int count, char *filepath)
{
	int nread, total = 0;
	FILE *fp;
	if (filepath != NULL)//需要将数据备份到本地文件中
	{
		//追加方式打开可读写文件filepath
		if ((fp = fopen(filepath, "a+") == NULL))
		{
			perror(filepath);
			return -1;
		}
	}
	while (total != count)
	{
		nread = read(fd, buffer, count - total);
		if (nread == 0)
		{
			printf("[Read]:%d Bytes\n", total);
			return total; /*读取队列为空，读取完毕*/
		}
		if (nread == -1)
		{
			printf("[Error]:read error!Maybe for the other side has close the connection\n");
			return -1; /*读出错，可能对方关关闭了连接*/
		}

		total += nread;
		buffer += nread;
		if (filepath != NULL)
		{
			if ((fputs(buffer, fp) == NULL))
			{
				perror("[file write error]");
				return -1;
			}
		}

	}
	printf("[Read]:%d Bytes\n", total);
	if (filepath != NULL)
		fclose(fp);
	return total; /*返回已经读取的字节数*/
}


/**********************
* 函数名称：SetSocketNonblock
* 功    能：设置非阻塞连接
* 参    数：fd--文件描述符
* 返    回：int - 0成功，-1失败
* 说    明：
***********************/
int SetSocketNonblock(int fd)
{
	int flags;
	if ((flags = fcntl(fd, F_GETFL, 0)) == -1) {
		perror("fcntl(fd,F_GETFL)");
		return -1;
	}
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		perror("fcntl(fd,F_SETFL,flags | O_NONBLOCK)");
		return -1;
	}
	return 0;
}