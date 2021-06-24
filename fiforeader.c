#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<fcntl.h>
#define MAXLINE 100
int readLine(int,char*);
int main(){
	int fd;
	char str[MAXLINE];
	mkfifo("myPipe",0660);
	fd = open("myPipe",O_RDONLY);
	while(readLine(fd,str))printf("%s\n",str);
	close(fd);
	return 0;
}
int readLine(int fd,char* str){
	int n;
	do{
		n=read(fd,str,1);
	}while(n>0 && *str++ != NULL);
	return(n>0);
}
