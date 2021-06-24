#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#define MAX_LINE 100
int main(){
	int n,pid,fd[2];
	char line[MAX_LINE];
	if((pid = fork()) == 0){
		close(fd[0]);
		dup2(fd[1],1);
		close(fd[1]);
		printf("Hello! pipe\n");
		printf("Bye! pipe\n");
	}
	else{
		close(fd[1]);
		printf("string from child process\n");
		while((n == read(fd[0],line,MAX_LINE)) > 0)write(STDOUT_FILENO,line,n);
	}
	exit(0);
}
