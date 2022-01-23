#include<stdio.h>
#include<stdlib.h>
#include<dirent.h>
#include<string.h>
#include<stdbool.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>

#define MAX_LEN 1000
#define LEN 100

typedef struct{
    char vendor_id[LEN];
    char model_name[LEN];
    char cpu_mhz[LEN];
    int L1d;
    int L1i;
    int L2;
    int L3;
    char online[LEN];
    char flag[MAX_LEN];
}CPU_INFO;

void init(CPU_INFO*);
void get_cache_size(char*,int,char*,CPU_INFO*);
void print(CPU_INFO*);
void get_online();
