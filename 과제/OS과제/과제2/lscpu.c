#include "mylscpuheader.h"

int main(){
    FILE *fp,*fp1;
    char buf[MAX_LEN];
    int cnt = 0;
    DIR *dir;
    struct dirent *diren;
    CPU_INFO mylscpu;

    init(&mylscpu);
    if((fp = fopen("/proc/cpuinfo","r")) == NULL){
        fprintf(stderr,"fopen error in /proc/cpuinfo\n");
        exit(1);
    }
    if((dir = opendir("/sys/devices/system/cpu")) == NULL){
        fprintf(stderr,"dir open error in /sys/devices/system/cpu");
        exit(1);
    }
    while(fgets(buf,MAX_LEN,fp)){
        if(strstr(buf,"vendor_id"))
        {
            strcpy(mylscpu.vendor_id, buf + 12);
            cnt++;
        }
        else if(strstr(buf,"model name"))
        {
            strcpy(mylscpu.model_name, buf + 13);
            cnt++;
        }
        else if(strstr(buf,"cpu MHz"))
        {
            strcpy(mylscpu.cpu_mhz, buf + 10);
            cnt++;
        }
        else if(strstr(buf,"flags"))
        {
            strcpy(mylscpu.flag, buf + 9);
            cnt++;
        }
        if(cnt == 4)break;
    }
    get_online(&mylscpu);
    while((diren = readdir(dir)) != NULL){
        if(!strstr(diren -> d_name,"cpu"))continue;
        if(diren -> d_name[3] < '0' || diren -> d_name[3] > '9')continue;
        struct stat statbuf;
        char path[MAX_LEN];
        int index_location;
        FILE *fptr;
        strcpy(path,"/sys/devices/system/cpu/");
        strcat(path,diren -> d_name);
        if(stat(path,&statbuf) < 0){
            fprintf(stderr,"stat error in %s\n",path);
            exit(1);
        }
        if(!S_ISDIR(statbuf.st_mode))continue;
        strcat(path,"/cache/index");
        strcat(path,"0");
        index_location = strlen(path) - 1;
        strcat(path,"/size");
        while((fptr = fopen(path,"r")) != NULL){
            fgets(buf,MAX_LEN,fptr);
            get_cache_size(path,index_location,buf,&mylscpu);
            path[index_location]++;
        }
    }
    
    print(&mylscpu);
}

void init(CPU_INFO *mylscpu){
    memset(mylscpu -> vendor_id,0,LEN);
    memset(mylscpu -> model_name,0,LEN);
    memset(mylscpu -> cpu_mhz,0,LEN);
    mylscpu -> L1d = 0;
    mylscpu -> L1i = 0;
    mylscpu -> L2 = 0;
    mylscpu -> L3 = 0;
    memset(mylscpu -> online,0,LEN);
    memset(mylscpu -> flag,0,MAX_LEN);
}

void get_cache_size(char *path,int index_location,char *buf,CPU_INFO *mylscpu){
    for(int i=0;i<strlen(buf);i++)
        if(buf[i] < '0' || buf[i] > '9'){
            buf[i] = '\0';
            break;
        }

    if(path[index_location] == '0')
        mylscpu -> L1d += atoi(buf);
    else if(path[index_location] == '1')
        mylscpu -> L1i += atoi(buf);
    else if(path[index_location] == '2')
        mylscpu -> L2 += atoi(buf);
    else if(path[index_location] == '3')
        mylscpu -> L3 += atoi(buf);
}

void print(CPU_INFO *ptr){
    printf("Vendor ID:			 %s",ptr -> vendor_id);
    printf("Model name:			 %s",ptr -> model_name);
    printf("CPU MHz:  	         	%s",ptr -> cpu_mhz);
    printf("L1d:				 %d KiB\n",ptr -> L1d);
    printf("L1i:				 %d KiB\n",ptr -> L1i);
    printf("L2:				 %d MiB\n",ptr -> L2 / 1024);
    printf("L3:				 %d MiB\n",ptr -> L3 / 1024);
    printf("On-line CPU(s) list:    	 %s",ptr -> online);
    printf("flags:				 %s",ptr -> flag);
}

void get_online(CPU_INFO *mylscpu){
    FILE *fp;
    char buf[LEN];
    if((fp = fopen("/sys/devices/system/cpu/online","r")) == NULL){
        fprintf(stderr,"fopen error in \n");
        exit(1);
    }
    fgets(buf,LEN,fp);
    strcpy(mylscpu -> online,buf);
}
