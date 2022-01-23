#include<stdio.h>
#include<string.h>
#include<unistd.h>
#define READ 0
#define WRITE 1
int main(int argc,char* argv[]){
	char str[1024];
	char* command1,* command2;
	int fd[2];
	printf("shell");
	fgets(str,sizeof(str),stdin);
	str[strlen(str)-1] = '\0';
	if(strchr(str,'|') != NULL){
		command1 = strtok(str,"|");
		command2 = strtok(NULL,"|");
	}
	pipe(fd);
	if(fork() == 0){
		close(fd[READ]);
		dup2(fd[WRITE],1);
		close(fd[WRITE]);
		execlp(command1,command1,NULL);
		perror("pipe");
	}
	else{
		close(fd[WRITE]);
		dup2(fd[READ],0);
		close(fd[READ]);
		execlp(command2,command2,NULL);
		perror("pipe");
	}
}
