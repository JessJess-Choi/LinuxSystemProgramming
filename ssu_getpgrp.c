#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

int main(){
	pid_t pgid;
	pid_t pid;
	pid = getpid();
	pgid = getpgrp();
	pid = fork();
	printf("pid %d, pgid : %d\n",getpid(),getpgid(pid));
	exit(0);
}
