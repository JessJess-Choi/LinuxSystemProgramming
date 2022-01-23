#include<stdio.h>
#include<signal.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>

pid_t* ptr_pid;

int main(int argc,char* argv[]){
    pid_t pid,parent = getpid();
    ptr_pid = (pid_t*)malloc(sizeof(pid_t)*atoi(argv[1]));
    for(int i=0;i<atoi(argv[1]);i++){
        pid = fork();
        if(pid == 0){
            while(1){
                sleep(5);
                printf("pid[%d] : %d\n",i,getpid());
            }
        }
        else if(pid > 0)
            ptr_pid[i]=pid;
    }
    if(getpid() == parent){
        sleep(5);
        for(int i=atoi(argv[1])-1;i>=0;i--){
            kill(ptr_pid[i],SIGKILL);
            printf("kill pid[%d] : %d\n",i,ptr_pid[i]);
            sleep(1);
        }
        printf("end\n");
    }
    return 0;
}