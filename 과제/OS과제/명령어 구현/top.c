#include "mytopheader.h"

int hertz,row,col;
unsigned long uptime,total_mem;
time_t now,before;
MY_PROC proc_list[MAX_LEN];
MY_PROC *sorted[MAX_LEN];
int proc_cnt;
char my_tty[SMALL_LEN];

int main(){
    unsigned long cpu_time[PID_MAX_LEN];
    pid_t pid = getpid();
    uid_t uid = getuid();
    char pid_path[SMALL_LEN],path[SMALL_LEN];

    now = time(NULL);

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

    initscr();
    halfdelay(10);
    noecho();
    keypad(stdscr,TRUE);
    curs_set(0);

    search_process(cpu_time);
    row = col = 0;

    int ch = 0;
    bool check = false;
    before = time(NULL);

    sort_cpu();
    print();
    refresh();

    do{
        now = time(NULL);
        switch(ch){
            case KEY_LEFT: 
                col--;
                if(col < 0)col = 0;
                check = true;
                break;
            case KEY_RIGHT : 
                col++;
                check = true;
                break;
            case KEY_UP :
                row--;
                if(row < 0)row = 0;
                check = true;
                break;
            case KEY_DOWN :
                row++;
                if(row > proc_cnt)row = proc_cnt;
                check = true;
                break;
        }
        if(check || now - before >= 3){
            erase();

            init_proc_list();
            search_process(cpu_time);
            sort_cpu();
            print();
            refresh();
            before = now;
            check = false;
        }
    }while((ch = getch()) != 'q');

    endwin();
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
        char path[BIG_LEN];
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
            memset(buf,0,sizeof(buf));
            fgets(buf,sizeof(buf),status_fp);
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

        long double mem = (long double)proc.rss / total_mem * 100;

        proc.mem_percent = mem;
    }
    fclose(status_fp);

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
    
    int min = total_time / 3600, sec = (total_time - min*3600) / 60;

    sprintf(proc.time,"%02d:%02d.%02lld",min,sec,total_time-min*3600-sec*60);
    
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
/*
long double round_double(long double src,int rdx){
    if(!rdx)return (long double)((unsigned long long)src);
    long double tmp = src;
    int value;

    for(int i=0;i<=rdx;i++)tmp *= 10;
    tmp = (unsigned long long)tmp % 10;
    tmp /= 10;
    tmp = (long double)((unsigned long long)tmp);

    if(value >= 5)tmp++;
    for(int i=0;i<rdx;i++)tmp /= 10;
    return tmp;
}
*/
void sort_cpu(){
    for(int i=0;i<proc_cnt;i++)sorted[i] = proc_list + i;
    for(int i=proc_cnt-1;i>0;i--)
        for(int j=0;j<i;j++)
            if(compare_cpu(sorted[j],sorted[j+1])){
                MY_PROC *tmp = sorted[j];
                sorted[j] = sorted[j+1];
                sorted[j+1] = tmp;
            }
}

bool compare_cpu(MY_PROC *a, MY_PROC *b){
    if(a -> cpu_percent < b -> cpu_percent)
        return true;
    else if(a -> cpu_percent > b -> cpu_percent)
        return false;
    else{
        if(a -> pid > b -> pid)return true;
        else
            return false;
    }
}

void print(){
    uptime = get_uptime();
    char buf[BIG_LEN];
    

    //uptime 출력
    char now_time[BUF_LEN];
    memset(now_time,0,sizeof(now_time));
    struct tm *tm_now = localtime(&now);
    sprintf(now_time,"top - %02d:%02d:%02d ",tm_now -> tm_hour,tm_now -> tm_min,tm_now -> tm_sec);

    struct tm *tm_now_uptime = localtime(&uptime);
    char now_uptime[BUF_LEN];
    memset(now_uptime,0,BUF_LEN);
    if(uptime < 60 * 60)sprintf(now_uptime,"%2d min",tm_now_uptime -> tm_min);
    else if(uptime < 60*60*24)sprintf(now_uptime, "%2d:%02d", tm_now_uptime->tm_hour, tm_now_uptime->tm_min);
    else
        sprintf(now_uptime, "%3d days, %02d:%02d", tm_now_uptime->tm_yday, tm_now_uptime->tm_hour, tm_now_uptime->tm_min);
    
    int count_user = 0;
    struct utmp *u_tmp;
    setutent();
    while((u_tmp = getutent()) != NULL)
        if(u_tmp -> ut_type == USER_PROCESS)
            count_user++;
    
    FILE *loadavg_fp;
    long double loadavg[3];
    if((loadavg_fp = fopen("/proc/loadavg","r")) == NULL){
        fprintf(stderr,"fopen error in /proc/loadavg\n");
        exit(1);
    }
    memset(buf,0,sizeof(buf));
    fgets(buf,sizeof(buf),loadavg_fp);
    fclose(loadavg_fp);
    sscanf(buf,"%Lf%LF%Lf",&loadavg[0],&loadavg[1],&loadavg[2]);

    mvprintw(TOP_ROW, 0, "%sup %s, %d users, load average: %4.2Lf, %4.2Lf, %4.2Lf", now_time, now_uptime, count_user, loadavg[0], loadavg[1], loadavg[2]);

    //task 출력
    unsigned int total = proc_cnt, running = 0, sleeping = 0, stopped = 0, zombie = 0;
    for(int i=0;i<proc_cnt;i++)
        if(!strcmp(proc_list[i].stat, "R"))running++;
        else if(!strcmp(proc_list[i].stat,"D"))sleeping++;
        else if(!strcmp(proc_list[i].stat,"S"))sleeping++;
        else if(!strcmp(proc_list[i].stat,"T"))stopped++;
        else if(!strcmp(proc_list[i].stat,"t"))stopped++;
        else if(!strcmp(proc_list[i].stat,"Z"))zombie++;

    mvprintw(TASK_ROW, 0, "Tasks:  %4u total,  %4u running, %4u sleeping,  %4u stopped, %4u zombie", total, running, sleeping, stopped, zombie);

    //cpu 출력
    FILE *cpu_fp;
    if((cpu_fp = fopen("/proc/stat","r")) == NULL){
        fprintf(stderr,"fopen error for /proc/stat\n");
        exit(1);
    }
    memset(buf,0,sizeof(buf));
    fgets(buf,sizeof(buf),cpu_fp);
    fclose(cpu_fp);

    char *ptr = buf;
    while(*ptr < '0' || *ptr > '9')ptr++;
    long double ticks[8],before_ticks[8];
    sscanf(ptr, "%Lf%Lf%Lf%Lf%Lf%Lf%Lf%Lf", ticks+0, ticks+1, ticks+2, ticks+3, ticks+4, ticks+5, ticks+6, ticks+7);

    unsigned long tick_cnt = 0;
    long double print_tick[8],before_uptime = 0.0;
    if(before_uptime){
        tick_cnt = (uptime - before_uptime) * hertz;
        for(int i=0;i<8;i++)
            print_tick[i] = ticks[i] - before_ticks[i];
    }
    else{
        tick_cnt = uptime * hertz;
        for(int i=0;i<8;i++)
            print_tick[i] = ticks[i];
    }
    for(int i=0;i<8;i++){
        print_tick[i] = (print_tick[i] / tick_cnt) * 100;

    }
     mvprintw(CPU_ROW,0,"%%Cpu(s):  %4.1Lf us, %4.1Lf sy, %4.1Lf ni, %4.1Lf id, %4.1Lf wa, %4.1Lf hi, %4.1Lf si, %4.1Lf st\n", print_tick[0], print_tick[2], print_tick[1], print_tick[3], print_tick[4], print_tick[5], print_tick[6], print_tick[7]);

    before_uptime = uptime;
    for(int i=0;i<8;i++)before_ticks[i] = ticks[i];

    //mem swap 출력
    FILE *mem_fp;
    if((mem_fp = fopen("/proc/meminfo","r")) == NULL){
        fprintf(stderr,"fopen error in /proc/meminfo\n");
        exit(1);
    }

    unsigned long mem_total,mem_free,mem_used,mem_available,buffer,cached,s_reclaimable,swap_total,swap_free,swap_used;
    int indx = 0;
    
    read_meminfo(mem_fp,&mem_total,&indx,1);
    read_meminfo(mem_fp,&mem_free,&indx,2);
    read_meminfo(mem_fp,&mem_available,&indx,3);
    read_meminfo(mem_fp,&buffer,&indx,4);
    read_meminfo(mem_fp,&cached,&indx,5);
    read_meminfo(mem_fp,&swap_total,&indx,15);
    read_meminfo(mem_fp,&swap_free,&indx,16);
    read_meminfo(mem_fp,&s_reclaimable,&indx,24);

    mem_used = mem_total - mem_free - buffer - cached - s_reclaimable;
    swap_used = swap_total - swap_free;

    mvprintw(MEM_ROW,0,"Mib Mem : %8.1Lf total,  %8.1Lf free,  %8.1Lf used,  %8.1Lf buff/cache\n", (long double)mem_total / 1024, (long double)mem_free / 1024, (long double)mem_used / 1024, (long double)(buffer+cached+s_reclaimable) / 1024);
    mvprintw(MEM_ROW+1,0,"Mib Swap: %8.1Lf total,  %8.1Lf free,  %8.1Lf used,  %8.1Lf avail Mem\n", (long double)swap_total / 1024, (long double)swap_free / 1024, (long double)swap_used / 1024, (long double)mem_available / 1024);



    fclose(mem_fp);

    int column_width[COLUMN_LEN] = {3, 4, 2, 2, 4, 3, 3, 1, 4, 3, 4, 7 };

    indx = 0;
    for(int i=0;i<proc_cnt;i++){               //pid의 최대 길이
        sprintf(buf,"%lu",proc_list[i].pid);
        column_width[indx] = (column_width[indx] > strlen(buf)) ? column_width[indx] : strlen(buf);
    }
    indx++;
    for(int i=0;i<proc_cnt;i++)             //user의 최대 길이
        column_width[indx] = (column_width[indx] > strlen(proc_list[i].user_name)) ? column_width[indx] : strlen(proc_list[i].user_name);
    indx++;
    for(int i=0;i<proc_cnt;i++){            //priority 최대 길이
        sprintf(buf,"%d",proc_list[i].priority);
        column_width[indx] = (column_width[indx] > strlen(buf)) ? column_width[indx] : strlen(buf);
    }
    indx++;
    for(int i=0;i<proc_cnt;i++){            //nice 최대 길이
        sprintf(buf,"%d",proc_list[i].nice);
        column_width[indx] = (column_width[indx] > strlen(buf)) ? column_width[indx] : strlen(buf);
    }
    indx++;
    for(int i=0;i<proc_cnt;i++){            //vsz 최대 길이
        sprintf(buf,"%lu",proc_list[i].vsz);
        column_width[indx] = (column_width[indx] > strlen(buf)) ? column_width[indx] : strlen(buf);
    }
    indx++;
    for(int i=0;i<proc_cnt;i++){            //rss 최대 길이
        sprintf(buf,"%lu",proc_list[i].rss);
        column_width[indx] = (column_width[indx] > strlen(buf)) ? column_width[indx] : strlen(buf);
    }
    indx++;
    for(int i=0;i<proc_cnt;i++){            //shr 최대 길이
        sprintf(buf,"%lu",proc_list[i].shr);
        column_width[indx] = (column_width[indx] > strlen(buf)) ? column_width[indx] : strlen(buf);
    }
    indx++;
    for(int i=0;i<proc_cnt;i++)            //stat 최대 길이
        column_width[indx] = (column_width[indx] > strlen(proc_list[i].stat)) ? column_width[indx] : strlen(proc_list[i].stat);
    indx++;
    for(int i=0;i<proc_cnt;i++){            //cpu 최대 길이
        sprintf(buf,"%3.1Lf",proc_list[i].cpu_percent);
        column_width[indx] = (column_width[indx] > strlen(buf)) ? column_width[indx] : strlen(buf);
    }
    indx++;
    for(int i=0;i<proc_cnt;i++){            //mem 최대 길이
        sprintf(buf,"%3.1Lf",proc_list[i].mem_percent);
        column_width[indx] = (column_width[indx] > strlen(buf)) ? column_width[indx] : strlen(buf);
    }
    indx++;
    for(int i=0;i<proc_cnt;i++)            //time 최대 길이
        column_width[indx] = (column_width[indx] > strlen(proc_list[i].time)) ? column_width[indx] : strlen(proc_list[i].time);
    indx++;
    for(int i=0;i<proc_cnt;i++)            //command 최대 길이
        column_width[indx] = (column_width[indx] > strlen(proc_list[i].cmd)) ? column_width[indx] : strlen(proc_list[i].cmd);


    int start_x[12] = {0,}, start_col = 0, end_col = 0, max_cmd = -1;
    
    if(col >= 11){
        start_col = 11;
        end_col = 12;
        max_cmd = COLS;
    }
    else{
        int i;
        for(i=col+1;i<12;i++){
            start_x[i] = column_width[i-1] + 2 + start_x[i-1];
            if(start_x[i] >= COLS){
                end_col = i;
                break;
            }
        }
        start_col = col;
        if(i == 12){
            end_col = 12;
            max_cmd = COLS - start_x[11];
        }
    }
    
    //column 출력
    attron(A_REVERSE);
    for(int i=0;i<COLS;i++)
        mvprintw(COLUMN_ROW,i," ");

    int gap = 0;
    if(start_col <= 0 && 0 < end_col){
        gap = column_width[0] - 3;   //pid 길이 차
        mvprintw(COLUMN_ROW,start_x[0]+gap,"%s","PID");
    }

    if(start_col <= 1 && 1 < end_col){
        mvprintw(COLUMN_ROW,start_x[1]+gap,"%s","USER");
    }

    if(start_col <= 2 && 2 < end_col){
        gap = column_width[2] - 2;
        mvprintw(COLUMN_ROW,start_x[2]+gap,"%s","PR");
    }
    
    if(start_col <= 3 && 3 < end_col){
        gap = column_width[3] - 2;
        mvprintw(COLUMN_ROW,start_x[3]+gap,"%s","NI");
    }
    
    //VIRT 출력
	if(start_col <= 4 && 4 < end_col){
		gap = column_width[4] - 4;	//VSZ의 길이 차 구함
		mvprintw(COLUMN_ROW, start_x[4] + gap, "%s", "VIRT");	//우측 정렬
	}

	if(start_col <= 5 && 5 < end_col){
		gap = column_width[5] - 3;	//RSS의 길이 차 구함
		mvprintw(COLUMN_ROW, start_x[5] + gap, "%s", "RES");	//우측 정렬
	}

	if(start_col <= 6 && 6 < end_col){
		gap = column_width[6] - 3;	//SHR의 길이 차 구함
		mvprintw(COLUMN_ROW, start_x[6] + gap, "%s", "SHR");	//우측 정렬
	}

	if(start_col <= 7 && 7 < end_col){
		mvprintw(COLUMN_ROW, start_x[7], "%s", "S");	//우측 정렬
	}

	if(start_col <= 8 && 8 < end_col){
		gap = column_width[8] - 4;	//CPU의 길이 차 구함
		mvprintw(COLUMN_ROW, start_x[8] + gap, "%s", "\%CPU");	//우측 정렬
	}

	if(start_col <= 9 && 9 < end_col){
		gap = column_width[9] - 3;	//MEM의 길이 차 구함
		mvprintw(COLUMN_ROW, start_x[9] + gap, "%s", "\%MEM");	//우측 정렬
	}

	if(start_col <= 10 && 10 < end_col){
		gap = column_width[10] - 5;	//TIME의 길이 차 구함
		mvprintw(COLUMN_ROW, start_x[10] + gap, "%s", "TIME+");	//우측 정렬
	}

    mvprintw(COLUMN_ROW,start_x[11],"%s","COMMAND");
    attroff(A_REVERSE);

    //process 출력
    char token[SMALL_LEN];
    memset(token,0,SMALL_LEN);
    for(int i=row;i<proc_cnt;i++){
        if(start_col <= 0 && 0 < end_col){
            memset(token,0,SMALL_LEN);
            sprintf(token,"%lu",sorted[i] -> pid);
            gap = column_width[0] - strlen(token);
            mvprintw(COLUMN_ROW+1+i-row,start_x[0]+gap,"%s",token);
        }

        if(start_col <= 1 && 1 < end_col){
            gap = column_width[1] - strlen(sorted[i] -> user_name);
            mvprintw(COLUMN_ROW+1+i-row,start_x[1],"%s",sorted[i] -> user_name);
        }

        if(start_col <= 2 && 2 < end_col){
            memset(token,0,sizeof(token));
            sprintf(token,"%d",sorted[i] -> priority);
            gap = column_width[2] - strlen(token);
            mvprintw(COLUMN_ROW+1+i-row,start_x[2]+gap,"%s",token);
        }

        if(start_col <= 3 && 3 < end_col){
            memset(token,0,sizeof(token));
            sprintf(token,"%d",sorted[i] -> nice);
            gap = column_width[3] - strlen(token);
            mvprintw(COLUMN_ROW+1+i-row,start_x[3]+gap,"%s",token);
        }

        if(start_col <= 4 && 4 < end_col){
            memset(token,0,sizeof(token));
            sprintf(token,"%lu",sorted[i] -> vsz);
            gap = column_width[4] - strlen(token);
            mvprintw(COLUMN_ROW+1+i-row,start_x[4]+gap,"%s",token);
        }

        if(start_col <= 5 && 5 < end_col){
            memset(token,0,sizeof(token));
            sprintf(token,"%lu",sorted[i] -> rss);
            gap = column_width[5] - strlen(token);
            mvprintw(COLUMN_ROW+1+i-row,start_x[5]+gap,"%s",token);
        }

        if(start_col <= 6 && 6 < end_col){
            memset(token,0,sizeof(token));
            sprintf(token,"%lu",sorted[i] -> shr);
            gap = column_width[6] - strlen(token);
            mvprintw(COLUMN_ROW+1+i-row,start_x[6]+gap,"%s",token);
        }

        if(start_col <= 7 && 7 < end_col){
            gap = column_width[7] - strlen(sorted[i] -> stat);
            mvprintw(COLUMN_ROW+1+i-row,start_x[7],"%s",sorted[i] -> stat);
        }

        if(start_col <= 8 && 8 < end_col){
            memset(token,0,sizeof(token));
            sprintf(token,"%3.1LF",sorted[i] -> cpu_percent);
            gap = column_width[8] - strlen(token);
            mvprintw(COLUMN_ROW+1+i-row,start_x[8]+gap,"%s",token);
        }

        if(start_col <= 9 && 9 < end_col){
            memset(token,0,sizeof(token));
            sprintf(token,"%3.1Lf",sorted[i] -> mem_percent);
            gap = column_width[9] - strlen(token);
            mvprintw(COLUMN_ROW+1+i-row,start_x[9]+gap,"%s",token);
        }

        if(start_col <= 10 && 10 < end_col){
            gap = column_width[10] - strlen(sorted[i] -> time);
            mvprintw(COLUMN_ROW+1+i-row,start_x[10],"%s",sorted[i] -> time);
        }

        int tap = col - 11;
        if((col == 11) && (strlen(sorted[i] -> cmd_option) < tap*8))
            continue;
        if(col < 11)tap = 0;
        sorted[i] -> cmd[max_cmd] = '\0';
        mvprintw(COLUMN_ROW+1+i-row,start_x[11],"%s",sorted[i] -> cmd + tap*8);
    }
}

void read_meminfo(FILE *mem_fp,unsigned long *mem,int *indx,int limit){
    char buf[BIG_LEN], *ptr;
    while(*indx < limit){
        memset(buf,0,sizeof(buf));
        fgets(buf,sizeof(buf),mem_fp);
        *indx = *indx + 1;
    }
    ptr = buf;
    while(*ptr < '0' || *ptr > '9')ptr++;
    sscanf(ptr,"%lu",mem);
}
