#include "common.h"
#define STUNO "1551445"
#define FILENAMELEN 64
#define TIMESTRLEN 30
#define BUFSIZE 100000
void getLocalTime(char s[]);
void getFileName(int pid, char fileName[]);
void writeToFile(int pid_tmp, char timestr[], char randData_bak, char fileName[]);
void communicate(int conn, int pid, int isfork, int socket_id);