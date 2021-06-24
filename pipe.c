#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#define MAXLINE 100
int main(int argc,char* argv[]){
	int n,pid,fd[2];
	char line[MAXLINE];
	pipe(fd);
	if((pid = fork()) == 0){
		close(fd[0]);
		dup2(fd[1],1);
		close(fd[1]);
		execvp(argv[1],&argv[1]);
	}
	else{
		close(fd[1]);
		printf("command results from child process\n");
		while((n=read(fd[0],line,MAXLINE)) > 0)write(STDOUT_FILENO,line,n);
	}
	exit(0);
}
