#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<sys/time.h>

#define MAXSIZE1 20000
#define MAXSIZE2 50000
#define MAXSIZE3 100000

void vector_calculate(int n){
	int arr[10][10],size;
	if(n == 0)size = MAXSIZE1;
	else if(n == 1)size = MAXSIZE2;
	else if(n == 2)size = MAXSIZE3;

	for(int i=0;i<size;i++)
		for(int j=0;j<size;j++)
			arr[i%10][j%10] = 10 * 10;
}

int main(){
	int status;
	struct timeval start,end;
	gettimeofday(&start,NULL);
	pid_t pid;
	for(int i=0;i<21;i++){
		if((pid = fork()) < 0){
			fprintf(stderr,"error in fork()\n");
			exit(1);
		}
		else if(pid == 0){
			printf("%d process starts\n",getpid());
			if(i/7 == 0)nice(10);
			else if(i/7 == 1)nice(0);
			else if(i/7 == 2)nice(-15);
			vector_calculate(i / 7);
			exit(0);
		}
	}
	for(int i=0;i<21;i++){
		pid = wait(&status);
		printf("%d process ends\n",pid);
		gettimeofday(&end,NULL);
		if(i%7 == 6)printf("========%ld=======\n",end.tv_sec - start.tv_sec);
	}
	printf("================All Processes End============================\n");
	gettimeofday(&end,NULL);
	printf("%ld\n",end.tv_sec - start.tv_sec);
	return 0;
}
