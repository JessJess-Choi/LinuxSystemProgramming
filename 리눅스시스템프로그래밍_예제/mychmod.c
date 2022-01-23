#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/stat.h>
#include<string.h>
#include<ctype.h>

void digit_mode(int,char**);
void english_mode(int,char**);


int main(int argc,char* argv[]){
    if(isdigit(argv[1][0]))
        digit_mode(argc,argv);
    else if(isalpha(argv[1][0]))
        english_mode(argc,argv);
    else{
        puts("error");
        exit(1);
    }
    exit(0);
}

void digit_mode(int argc,char* argv[]){
    struct stat statbuf;
    if(stat(argv[2],&statbuf) < 0){
        puts("error");
        exit(1);
    }
    int bit=0;
        if(strlen(argv[1]) == 3){
            for(int i=0;i<3;i++){
                int tmp = argv[1][i] - '0';
                bit += tmp << ((2-i)*3);
            }
        }
        else{
            bit = (argv[1][0] - '0') << 9;
            for(int i=1;i<strlen(argv[1]);i++){
                int tmp = argv[1][i] - '0';
                bit += tmp << ((3-i)*3);
            }
        }
        if(chmod(argv[2],bit) < 0){
                puts("error");
                exit(1);
            }
}

void english_mode(int argc,char* argv[]){
    char* token = strtok(argv[1],",");
    while(token != NULL){
        printf("%s\n",token);
        token = strtok(NULL,",");
    }
}