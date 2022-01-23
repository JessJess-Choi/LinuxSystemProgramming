#include "mypsheader.h"

int hertz;
unsigned long uptime,total_mem;
time_t before;
MY_PROC proc_list[MAX_LEN];
MY_PROC *sorted[MAX_LEN];
int proc_cnt;
char my_tty[SMALL_LEN];
uid_t my_uid;

int main(){
    unsigned long cpu_time[PID_MAX_LEN];
    pid_t pid = getpid();
    my_uid = getuid();
    char pid_path[SMALL_LEN],path[SMALL_LEN];

    total_mem = get_total_memory();
    hertz = sysconf(_SC_CLK_TCK);

    memset(cpu_time,0,sizeof(cpu_time));
    memset(pid_path,0,sizeof(pid_path));
    memset(path,0,sizeof(path));
    sprintf(pid_path,"/%d",pid);
    strcpy(path,"/proc");
    strcat(path,pid_path);

    get_tty(path,my_tty);
    for(int i=4;i<strlen(my_tty);i++)
        if(my_tty[i] < '0' || my_tty[i] >'9'){
            my_tty[i] = '\0';
            break;
        }

    search_process(cpu_time);

    print();
    return 0;
}

unsigned long get_total_memory(){
    FILE *fp;
    char buf[BIG_LEN];
    char *ptr = buf;
    unsigned long ret;

    if((fp = fopen("/proc/meminfo","r")) == NULL){
        fprintf(stderr,"fopen error in /proc/meminfo \n");
        exit(1);
    }

    memset(buf,0,BIG_LEN);
    fgets(buf,BIG_LEN,fp);      //memtotal read

    while(*ptr < '0' || *ptr > '9')ptr++;
    sscanf(ptr,"%lu",&ret);
    fclose(fp);
    ret = to_kb(ret);
    return ret;
}

unsigned long to_kb(unsigned long ul){
    unsigned long ret = ul * 1024 / 1000;
    return ret;
}

void get_tty(char path[BIG_LEN],char tty[SMALL_LEN]){
    char zero_path[BIG_LEN],tmp[BIG_LEN];
    memset(tty,0,SMALL_LEN);
    memset(zero_path,0,sizeof(zero_path));
    strcpy(zero_path,path);
    strcat(zero_path,"/fd/0");

    if(access(zero_path,F_OK) < 0){
        char stat_path[SMALL_LEN],now_path[BIG_LEN];
        FILE *stat_fp;
        int tty_nr;
        DIR *dir;
        struct dirent *diren;

        memset(stat_path,0,sizeof(stat_path));
        strcpy(stat_path,path);
        strcat(stat_path,"/stat");

        if((stat_fp = fopen(stat_path,"r")) == NULL){
            fprintf(stderr,"fopen error in %s\n",stat_path);
            return;
        }

        for(int i=0;i<=STAT_TTY_NR;i++){
            memset(tmp,0,sizeof(tmp));
            fscanf(stat_fp,"%s",tmp);
        }
        fclose(stat_fp);
        
        tty_nr = atoi(tmp);

        if((dir = opendir("/dev")) == NULL){
            fprintf(stderr,"open directory error in /dev");
            exit(1);
        }

        while((diren = readdir(dir)) != NULL){
            struct stat statbuf;

            memset(now_path,0,sizeof(now_path));
            strcpy(now_path,"/dev/");
            strcat(now_path,diren -> d_name);

            if(stat(now_path,&statbuf) < 0){
                fprintf(stderr,"stat error in %s\n",now_path);
                exit(1);
            }
            if(!S_ISCHR(statbuf.st_mode))continue;
            else if(statbuf.st_rdev == tty_nr){
                strcpy(tty,diren -> d_name);
                break;
            }
        }
        closedir(dir);

        if(!strlen(tty))strcpy(tty,"?");
    }
    else{
        char symbolic[BIG_LEN];
        memset(symbolic,0,sizeof(symbolic));

        if(readlink(zero_path,symbolic,BIG_LEN) < 0){
            fprintf(stderr,"symbolic link error in %s\n",zero_path);
            exit(1);
        }
        if(!strcmp(symbolic,"/dev/null"))strcpy(tty,"?");
        else
            sscanf(symbolic,"/dev/%s",tty);
    }
}

void search_process(unsigned long cpu_time[PID_MAX_LEN]){
    uptime = get_uptime();
    DIR *dir;
    struct dirent *diren;

    if((dir = opendir("/proc")) == NULL){
        fprintf(stderr,"error in opening /proc\n");
        exit(1);
    }
    while((diren = readdir(dir)) != NULL){
        char path[BIG_LEN],tty[BIG_LEN];
        struct stat statbuf;
        bool check = false;

        memset(path,0,BIG_LEN);
        strcpy(path,"/proc/");
        strcat(path,diren -> d_name);
        if(stat(path,&statbuf) < 0){
            fprintf(stderr,"stat error in %s\n",path);
            exit(1);
        }
        if(!S_ISDIR(statbuf.st_mode))continue;
        
        for(int i=0;i<strlen(diren->d_name);i++)
            if(diren -> d_name[i] < '0' || diren -> d_name[i] > '9'){
                check = true;
                break;
            }
        if(check)continue;
        if(statbuf.st_uid != my_uid)continue;
        
        memset(tty,0,BIG_LEN);
        get_tty(path,tty);
        if(!strlen(tty) || !strcmp(tty,"?"))continue;
        if(strcmp(tty,my_tty))continue;

        add_proc_list(path,cpu_time);
    }
    closedir(dir);
}

unsigned long get_uptime(){
    FILE *fp;
    char buf[BIG_LEN];
    long double time;

    memset(buf,0,sizeof(buf));

    if((fp = fopen("/proc/uptime","r")) == NULL){
        fprintf(stderr,"fopen error in /proc/uptime\n");
        exit(1);
    }
    fgets(buf,sizeof(buf),fp);
    sscanf(buf,"%Lf",&time);
    fclose(fp);
    return (unsigned long)time;
}

//pid 디렉토리 내의 파일로 MY_PROC 구조체 완성
void add_proc_list(char path[BIG_LEN],unsigned long cpu_time[PID_MAX_LEN]){
    FILE *fp,*status_fp;
    MY_PROC proc;
    char stat_path[BIG_LEN], stat_token[SMALL_LEN][SMALL_LEN],tmp[SMALL_LEN],status_path[SMALL_LEN];
    struct stat statbuf;
    struct passwd *upasswd;
    unsigned long utime,stime,start_time,vmlck = 0;
    unsigned long long total_time;
    long double cpu = 0.0;
    bool check = true;

    strcpy(stat_path,path);
    strcat(stat_path,"/stat");
    
    if(access(stat_path,R_OK) < 0){
        fprintf(stderr,"access error in %s\n",stat_path);
        return;
    }

    init_proc(&proc);
    if((fp = fopen(stat_path,"r")) == NULL){
        fprintf(stderr,"fopen error in %s\n",stat_path);
        return;
    }
    memset(stat_token,0,sizeof(stat_token));
    for(int i=0;i<SMALL_LEN;i++)
        fscanf(fp,"%s",stat_token[i]);
    fclose(fp);

    proc.pid = atol(stat_token[STAT_PID]);

    if(stat(stat_path,&statbuf) < 0){
        fprintf(stderr,"stat error in %s\n",stat_path);
        return;
    }
    proc.uid = statbuf.st_uid;
    upasswd = getpwuid(statbuf.st_uid);

    strcpy(tmp,upasswd -> pw_name);
    tmp[8] = '\0';
    if(!strcmp(tmp,"systemd-")){
        tmp[7] = '+';
        strcpy(proc.user_name,tmp);
    }
    else
        strcpy(proc.user_name,upasswd -> pw_name);      //user 이름이 systemd-
                                                        //로 시작하면 systemd+를 유저명으로 저장
    
    utime = (unsigned long)atol(stat_token[STAT_UTIME]);
    stime = (unsigned long)atol(stat_token[STAT_STIME]);
    start_time = (unsigned long)atol(stat_token[STAT_START_TIME]);
    total_time = utime + stime;
    cpu_time[proc.pid] = total_time;

    cpu = (total_time / hertz) / (long double)(uptime - (start_time / hertz)) * 100;


    proc.cpu_percent = cpu;

    memset(status_path,0,sizeof(status_path));
    strcpy(status_path,path);
    strcat(status_path,"/status");

    if((status_fp = fopen(status_path,"r")) == NULL){
        fprintf(stderr,"fopen error in %s\n",status_path);
        return;
    }

    char buf[BIG_LEN];
    int cnt = 0;
    while(cnt < STATUS_VSZ_ROW){
        memset(buf,0,sizeof(buf));
        fgets(buf,sizeof(buf),status_fp);
        cnt++;
    }
    char *ptr = buf;
    for(int i=0;i<strlen(buf);i++)
        if(buf[i] >= '0' && buf[i] <= '9'){
            ptr = &buf[i];
            break;
        }
    buf[6] = '\0';      // /proc/pid/status에서 VmSize가 있는지 확인
    if(strcmp(buf,"VmSize")){
        proc.vsz = 0;
        proc.rss = 0;
        proc.shr = 0;
        proc.mem_percent = 0.0;
    }
    else{
        sscanf(ptr,"%lu",&proc.vsz);
        while(cnt < STATUS_VMLCK_ROW){
            memset(buf,0,BIG_LEN);
            fgets(buf,BIG_LEN,status_fp);
            cnt++;
        }
        ptr = buf;
        for(int i=0;i<strlen(buf);i++)
            if(buf[i] >= '0' && buf[i] <='9'){
                ptr = &buf[i];
                break;
            }
        vmlck = 0;
        sscanf(ptr,"%lu",&vmlck);

        while(cnt < STATUS_RSS_ROW){
            memset(buf,0,sizeof(buf));
            fgets(buf,sizeof(buf),status_fp);
            cnt++;
        }
        ptr = buf;
        for(int i=0;i<strlen(buf);i++)
            if(buf[i] >= '0' && buf[i] <= '9'){
                ptr = &buf[i];
                break;
            }
        sscanf(ptr,"%lu",&proc.rss);

        while(cnt < STATUS_SHR_ROW){
            memset(buf,0,sizeof(buf));
            fgets(buf,sizeof(buf),status_fp);
            cnt++;
        }
        ptr = buf;
        for(int i=0;i<strlen(buf);i++)
            if(buf[i] >= '0' && buf[i] <= '9'){
                ptr = &buf[i];
                break;
            }
        sscanf(ptr,"%lu",&proc.shr);

        proc.mem_percent = (long double)proc.rss / total_mem * 100;
    }
    fclose(status_fp);

    get_tty(path,proc.tty);
    proc.priority = atoi(stat_token[STAT_PRIORITY]);
    proc.nice = atoi(stat_token[STAT_NICE]);
    strcpy(proc.stat,stat_token[STAT_STATE]);  

    unsigned long start = time(NULL) - uptime + (start_time / hertz);
    struct tm *tm_start = localtime(&start);
    if(time(NULL) - start < 24 * 60 * 60)
        strftime(proc.start,SMALL_LEN,"%H:%M",tm_start);
    else if(time(NULL) - start < 7 * 24 * 60 * 60)
        strftime(proc.start,SMALL_LEN,"%b %d",tm_start);
    else
        strftime(proc.start,SMALL_LEN,"%y",tm_start);
    
    unsigned long cpuTime = total_time / hertz;
    struct tm *tm_cpu_time = localtime(&cpuTime);
    
    int hour = total_time / 60, min = total_time / 3600, sec = (total_time - min*3600) / 60;
    //check//
    sprintf(proc.time,"%02d:%02d:%02d",hour,min,sec);

    sscanf(stat_token[STAT_CMD],"(%s",proc.cmd);
    proc.cmd[strlen(proc.cmd) - 1] = '\0';

    char cmd_line_path[BIG_LEN];
    memset(cmd_line_path,0,sizeof(cmd_line_path));
    strcpy(cmd_line_path,path);
    strcat(cmd_line_path,"/cmdline");
    FILE *cmd_line_fp;

    if((cmd_line_fp = fopen(cmd_line_path,"r")) == NULL){
        fprintf(stderr,"fopen error in %s\n",cmd_line_path);
        return;
    }

    while(1){
        char arr[2] = {'\0','\0'};
        fread(&arr[0],1,1,cmd_line_fp);
        if(arr[0] == '\0'){
            fread(&arr[0],1,1,cmd_line_fp);
            if(arr[0] == '\0')break;
            else
                strcat(proc.cmd_option,arr);
        }
        strcat(proc.cmd_option,arr);
    }
    if(!strlen(proc.cmd_option))sprintf(proc.cmd_option,"[%s]",proc.cmd);
    fclose(cmd_line_fp);

    proc_list[proc_cnt].pid = proc.pid;
    proc_list[proc_cnt].uid = proc.uid;
    strcpy(proc_list[proc_cnt].user_name,proc.user_name);
    proc_list[proc_cnt].cpu_percent = proc.cpu_percent;
    proc_list[proc_cnt].mem_percent = proc.mem_percent;
    proc_list[proc_cnt].vsz = proc.vsz;
    proc_list[proc_cnt].rss = proc.rss;
    proc_list[proc_cnt].shr = proc.shr;
    proc_list[proc_cnt].priority = proc.priority;
    proc_list[proc_cnt].nice = proc.nice;
    strcpy(proc_list[proc_cnt].tty,proc.tty);
    strcpy(proc_list[proc_cnt].stat,proc.stat);
    strcpy(proc_list[proc_cnt].time,proc.time);
    strcpy(proc_list[proc_cnt].cmd,proc.cmd);
    strcpy(proc_list[proc_cnt].cmd_option,proc.cmd_option);
    proc_cnt++;
}

//proc 구조체 초기화
void init_proc(MY_PROC *proc){
    proc -> pid = 0;
    proc -> uid = 0;
    memset(proc -> user_name,0,SMALL_LEN);
    proc -> cpu_percent = 0.0;
    proc -> mem_percent = 0.0;
    proc -> vsz = 0;
    proc -> rss = 0;
    proc -> shr = 0;
    proc -> priority = 0;
    proc -> nice = 0;
    memset(proc -> tty,0,SMALL_LEN);
    memset(proc -> stat,0,SMALL_LEN);
    memset(proc -> time,0,SMALL_LEN);
    memset(proc -> cmd,0,BIG_LEN);
    memset(proc -> cmd_option,0,BIG_LEN + 100);
}

void init_proc_list(){
    for(int i=0;i<proc_cnt;i++)
    {
        sorted[i] = NULL;
        init_proc(proc_list+i);
    }  
    proc_cnt = 0;
}

void print(){
    int column_width[COLUMN_LEN] = {4, 3, 4, 3, 3, 3, 3, 4, 5, 5, 3, 7 };
    int indx = 0;
    char buf[BIG_LEN];

    //column
    memset(buf,0,BIG_LEN);

    strcat(buf,"    PID TTY       TIME CMD\0");
    printf("%s\n",buf);

    //process 출력
    char token[SMALL_LEN];
    memset(token,0,SMALL_LEN);
    for(int i=0;i<proc_cnt;i++){
        memset(buf,0,BIG_LEN);
        memset(token,0,SMALL_LEN);
        sprintf(token,"  %lu",proc_list[i].pid);
        strcat(buf,token);
        
        strcat(buf," ");
        strcat(buf,proc_list[i].tty);
        strcat(buf,"    ");

        strcat(buf,proc_list[i].time);
        strcat(buf," ");
        
        strcat(buf,proc_list[i].cmd);
        printf("%s\n",buf);
    }
}
