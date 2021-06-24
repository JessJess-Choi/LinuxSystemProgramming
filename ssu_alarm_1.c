#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>

void ssu_signal_handler(int);

int count;

int main(){
	signal(SIGALRM,ssu_signal_handler);
	alarm(1);
	while(1);
	exit(0);
}
void ssu_signal_handler(int signo){
	printf("alarm %d\n",count++);
	alarm(1);
}
