#include<stdio.h>
#include<linux/kernel.h>
#include<sys/syscall.h>
#include<unistd.h>
#include<string.h>
#include<ctype.h>
#include<stdbool.h>

void parse_token(char*,int*,int*,char*);

int main(){
	int a,b,ans,*ptr;
	char ch;
	char buffer[1024];
	int n;
	printf("input number : ");
	scanf("%d\n",&n);
	while(n--){
		memset(buffer,0,sizeof(buffer));
		gets(buffer);
		parse_token(buffer,&a,&b,&ch);
		switch(ch){
			case '+':
				syscall(442,a,b,ptr);
				break;
			case '-':
				syscall(443,a,b,ptr);
				break;
			case '*':
				syscall(444,a,b,ptr);
				break;
			case '%':
				syscall(445,a,b,ptr);
				break;
		}
		printf("%d %c %d =  %d\n",a,ch,b,*ptr);
	}
	return 0;
}

void parse_token(char *buffer,int *a,int *b,char *ch){
	int index = -1;
	bool check = false;
	*a = *b = 0;
	for(int i=0;i<strlen(buffer);i++){
		if(i == 0 && buffer[i] == '-'){
			check = true;
			continue;
		}
		if(!isdigit(buffer[i])){
			*ch = buffer[i];
			index = i + 1;
			break;
		}
		else{
			*a = *a * 10 + buffer[i] - '0';
		}
	}
	if(check)
		*a *= -1;
	check = false;
	if(buffer[index] == '-'){
		check = true;
		index++;
	}
	for(int i=index;i<strlen(buffer);i++)
		*b = *b * 10 + buffer[i] - '0';
	if(check)
		*b *= -1;
}
