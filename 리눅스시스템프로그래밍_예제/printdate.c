#include<stdio.h>
#include<signal.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>

void handler1(int);

int main(int argc,char* argv[]){
    char* args[] = {"date",NULL};
    pid_t pid;
    signal(SIGINT,handler1);
  
    if(argc == 1){
        while(1){    
            pid = fork();
            if(pid == 0)
                execv("/bin/date",args);
            sleep(1);
        }
    }
    else if(argc == 2){
        int i=atoi(argv[1]);
        int left_time = i - 5;
        while(1){
            pid = fork();
            if(pid == 0)execv("/bin/date",args);
            if(i <= 5)printf("%d\n",i);
            i--;
            sleep(1);
                        if(i == 0)kill(getpid(),SIGKILL);
        }
    }
    return 0;
}

void handler1(int signo){
    printf("signal changed SIGINT to SIGKILL\n");
    kill(getpid(),SIGKILL);
}