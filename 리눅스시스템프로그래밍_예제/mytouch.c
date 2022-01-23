#include<stdio.h>
#include<sys/stat.h>
#include<unistd.h>
#include<fcntl.h>
#include<utime.h>
#include<stdlib.h>

int main(int argc,char* argv[]){
    struct stat statbuf;
    struct utimbuf time_buf;
    int fd;
    if(argc == 2){
        if((fd = open(argv[1], O_RDWR | O_TRUNC)) < 0){
            puts("error");
            exit(1);
        }
        time_buf.actime = statbuf.st_atime;
        time_buf.modtime = statbuf.st_mtime;
    }
    else{
        puts("error");
        exit(1);
    }
}