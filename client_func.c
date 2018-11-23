#include "client_func.h"
/**********************
* 函数名称：getLocalTime
* 功    能：获得当前进程的系统时间，以yyyy-mm-dd hh:mm:ss的格式
* 参    数：函数运行完成后s即结果字符串
* 返    回：
* 说    明：以1900.01.01作为时间基准
***********************/
void getLocalTime(char s[])
{
	time_t ttime;
	ttime = time(NULL);
	struct tm *stTime = localtime(&ttime);
	sprintf(s, "%d-%02d-%02d %02d:%02d:%02d\n",
		stTime->tm_year + 1900,
		stTime->tm_mon + 1,
		stTime->tm_mday,
		stTime->tm_hour,
		stTime->tm_min,
		stTime->tm_sec);
}

/**********************
* 函数名称：getFileName
* 功    能：根据所有者学号STUNO，进程号pid，返回满足要求的文件名
* 参    数：
* 返    回：
* 说    明：学号.进程号.pid.txt
***********************/
void getFileName(int pid, char fileName[])
{
	memset(fileName, 0, FILENAMELEN);
	char pid_str[10];
	itoa(pid, pid_str, 10);
	strcpy(fileName, STUNO);
	strcat(fileName, ".");
	strcat(fileName, pid_str);
	strcat(fileName, ".pid.txt");
}

/**********************
* 函数名称：writeToFile
* 功    能：将client端发送的stuno,pid,time,以及随机生成的数据写入指定文件
* 参    数：pid_tmp--网络序的pid号	timestr--指定格式(yyyy-mm-dd hh:mm:ss)的时间字符串
randData_bak--client端生成并发送的随机数据	fileName--要写入的文件名
* 返    回：
* 说    明：四行，注意换行符用linux下的格式
***********************/
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

/**********************
* 函数名称：communicate
* 功    能：与server端的通信流程
* 参    数：conn--连接套接字	pid--进程号
isfork--是否以fork子进程的方式创建连接
socket_id--第几个连接
* 返    回：
* 说    明：数据交互完成后，将数据写入指定文件
***********************/
void communicate(int conn, int pid, int isfork, int socket_id)
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

	//?????????????????????????????????????怎么正确的转网络序，不清楚，之后再改
	int stuno = htonl(atoi(STUNO));//将STUNO宏变量字符串转成int型后，转成网络序
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

	//?????????????????????????????????????怎么正确的转网络序，不清楚，之后再改
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
	char timestr[TIMESTRLEN];
	memset(timestr, 0, TIMESTRLEN);
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
	strncpy(tmp, readBuf + 3, 5);//截取后五位随机数
	int randNumber = atoi(tmp);
	if (strncmp(readBuf, "str", 3) != 0 | randNumber<32768)//接收到的信息不等于"str*****"
	{
		close(conn);//中断连接
		printf("Connection_%d closed!", pid);
		exit(0);
	}

	//生成随机序列
	//备份，用于后续写入文件
	//可优化，使用动态申请空间方式
	char randData_bak[BUFSIZE];
	for (int i = 0; i < randNumber; i++)
	{
		writeBuf[i] = rand() % 256;
		randData_bak[i] = writeBuf[i];
	}
	writeBuf[randNumber] = '\0';
	randData_bak[randNumber] = '\0';

	//为防止数据量过大，循环write
	int nwritten, total = 0;
	while (total != randNumber)
	{
		nwritten = write(conn, writeBuf, randNumber - total);
		if (nwritten == 0)  //这里如果设置了非阻塞写入的话，有可能是sendbuffer满了
		{
			printf("[sendbuffer may be full]\n");
			break;
		}
		if (nwritten == -1) //写入出错，可能对方关关闭了连接
		{
			perror("[write error]");
			exit(-1);
		}
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
	char fileName[FILENAMELEN];
	getFileName(pid, fileName);
	writeToFile(pid, timestr, randData_bak, fileName);
}