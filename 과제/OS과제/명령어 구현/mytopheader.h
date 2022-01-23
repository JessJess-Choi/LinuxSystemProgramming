#include<stdio.h>
#include<unistd.h>
#include<dirent.h>
#include<stdlib.h>
#include<time.h>
#include<limits.h>
#include<pwd.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<errno.h>
#include<stdbool.h>
#include<utmp.h>
#include<math.h>
#include<float.h>
#include<ncurses.h>

#define SMALL_LEN 32
#define BUF_LEN 128
#define BIG_LEN 1024
#define MAX_LEN 4096
#define PID_MAX_LEN 32768

#define STAT_PID 0
#define STAT_CMD 1
#define STAT_STATE 2
#define STAT_SID 5
#define STAT_TTY_NR 6
#define STAT_TPGID 7
#define STAT_UTIME 13
#define STAT_STIME 14
#define STAT_PRIORITY 17
#define STAT_NICE 18
#define STAT_N_THREAD 19
#define STAT_START_TIME 21

#define COLUMN_LEN 12

#define STATUS_VSZ_ROW 18
#define STATUS_VMLCK_ROW 19
#define STATUS_RSS_ROW 22
#define STATUS_SHR_ROW 24

#define COLS 70

#define TOP_ROW 0
#define TASK_ROW 1
#define CPU_ROW 2
#define MEM_ROW 3
#define COLUMN_ROW 6

typedef struct {
    unsigned long pid;
    unsigned long uid;
    char user_name[SMALL_LEN];
    long double cpu_percent;
    long double mem_percent;
    unsigned long vsz;
    unsigned long rss;
    unsigned long shr;
    int priority;
    int nice;
    char tty[SMALL_LEN];
    char stat[SMALL_LEN];
    char start[SMALL_LEN];
    char time[SMALL_LEN];
    char cmd[BIG_LEN];
    char cmd_option[BIG_LEN + 100];
}MY_PROC;

unsigned long get_total_memory();
void get_tty(char*,char*);
void search_process(unsigned long cpu_time[PID_MAX_LEN]);
unsigned long get_uptime();
void add_proc_list(char path[BIG_LEN],unsigned long cpu_time[PID_MAX_LEN]);
void init_proc(MY_PROC *proc);
void init_proc_list();
long double round_double(long double src, int rdx);
void sort_cpu();
bool compare_cpu(MY_PROC *a,MY_PROC *b);
void print();
void read_meminfo(FILE*,unsigned long*,int*,int);
unsigned long to_kb(unsigned long);

