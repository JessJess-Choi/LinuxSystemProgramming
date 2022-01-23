#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/wait.h>
#include<pthread.h>
#include<sys/time.h>

int living_cell_count(char**,int,int,int,int);
void get_next_generation_matrix(char**,char**,int,int,int);
void *thread_func(void*);

typedef struct thread_data{
    char** matrix;
    char** next_generation_matrix;
    char* filename;
    int* fd;
    int m;
    int n;
    int myindex;
    int handle_number;
}thread_data;

int main(int argc,char* argv[]){
    int N,generation_count=0,tmp,fd_old_generation,fd_generation,generation_number;
    char generation_filename[20];
    char** matrix,** next_generation_matrix;
    int m=0,n=0,process_number,thread_number;
    struct timeval start,end;
    char tmp_buffer[20010];
    pid_t* pid;

    while(1){
        int fd, handle_number;
        handle_number = 0;
        puts("=================input number=================");
        puts("1 :   exit");
        puts("2 :   sequential");
        puts("3 :   process");
        puts("4 :   thread");
        scanf("%d",&N);getchar();
        if(N==1)break;
        puts("==========input number of generation==========");
        scanf("%d",&generation_number);getchar();                   //몇세대 반복할건지 입력받음

        switch(N){
            case 2:
            gettimeofday(&start,NULL);
            fd = open(argv[1],O_RDONLY);
            read(fd,tmp_buffer,20010);
            for(int i=0;i<20010;i++)
                if(tmp_buffer[i] == '\n'){
                    n=i+1;
                    break;
                }
            lseek(fd,0,SEEK_SET);

            if(!m)                      //맨 처음에만 m값을 정해야 함
                while(1){
                    if((tmp = read(fd,tmp_buffer,n)) == 0)break;
                    m++;    
                }

            matrix = (char**)malloc(sizeof(char*)*m);
            for(int i=0;i<m;i++)matrix[i] = (char*)malloc(sizeof(char)*(n/2));
            next_generation_matrix = (char**)malloc(sizeof(char*)*m);
            for(int i=0;i<m;i++)next_generation_matrix[i] = (char*)malloc(sizeof(char)*(n/2));

            lseek(fd,0,SEEK_SET);
            for(int i=0;i<m;i++){
                read(fd,tmp_buffer,n);
                for(int j=0;j<n;j+=2){
                    matrix[i][j/2] = tmp_buffer[j];
                }
            }
            close(fd);
            n/=2;
            generation_count = 0;

            while(generation_number--){        //입력받은 세대 수 만큼 반복
                if(generation_count){       //1세대 이상이면 기록된 파일 읽어오기
                    sprintf(generation_filename,"%s%d%s","gen_",generation_count,".matrix");
                    fd_old_generation = open(generation_filename,O_RDONLY);
                    for(int i=0;i<m;i++){
                        read(fd_old_generation,tmp_buffer,n*2);   //tmp_buffer에 행렬의 한 행을 읽어옴
                        for(int j=0;j<2*n;j+=2)
                            matrix[i][j/2] = tmp_buffer[j];
                                                           //matrix에는 ' '와 '\n' 제외하고 저장
                    }
                    close(fd_old_generation);
                }
                sprintf(generation_filename,"%s%d%s","gen_",++generation_count,".matrix");
                fd_generation = open(generation_filename,O_WRONLY|O_CREAT|O_TRUNC,0777);
                                                //진행할 세대의 파일을 생성 및 열기
                for(int i=0;i<m;i++)
                    get_next_generation_matrix(matrix,next_generation_matrix,i,m-1,n-1);
                                //다음 세대의 세포에 관한 정보를 next_generation_matrix에 저장

                matrix = next_generation_matrix;
                next_generation_matrix = (char**)malloc(sizeof(char*)*m);
                for(int i=0;i<m;i++)next_generation_matrix[i] = (char*)malloc(sizeof(char)*2*n);
                        //다음 세대 정보를 matrix에 저장하고 next_generation_matrix는 새로운 메모리공간 할당
                
                for(int i=0;i<m;i++){
                    for(int j=0;j<2*n;j+=2){
                        next_generation_matrix[i][j] = matrix[i][j/2];
                        if(j == 2*n-2)next_generation_matrix[i][j+1] = '\n';
                        else
                            next_generation_matrix[i][j+1] = ' ';
                    }
                }           //next_generation_matrix에 ' '와 '\n'도 포함된 다음 세대 행렬 기록
                
                for(int i=0;i<m;i++)
                    for(int j=0;j<2*n;j++){
                        write(fd_generation,&next_generation_matrix[i][j],sizeof(next_generation_matrix[i][j]));
                    }
                close(fd_generation);
            }           //next_generation_matrix에 기록된 내용을 gen_n.matrix 파일에 출력

            rename(generation_filename,"output.matrix");    //마지막세대 파일은 output.matrix로 변경
            gettimeofday(&end,NULL);
            printf("%f ms\n",((double)end.tv_sec-start.tv_sec)*1000+((double)end.tv_usec-start.tv_usec)/1000);                //시간 측정 및 출력
            break;
            
            
            case 3:
            printf("=========input the number of process==========\n");
            scanf("%d",&process_number);getchar();
            gettimeofday(&start,NULL);
            fd = open(argv[1],O_RDONLY);
            read(fd,tmp_buffer,20010);
            for(int i=0;i<20010;i++)
                if(tmp_buffer[i] == '\n'){
                    n=i+1;
                    break;
                }
            lseek(fd,0,SEEK_SET);

            if(!m)                      //맨 처음에만 m값을 정해야 함
                while(1){
                    if((tmp = read(fd,tmp_buffer,n)) == 0)break;
                    m++;    
                }

            matrix = (char**)malloc(sizeof(char*)*m);
            for(int i=0;i<m;i++)matrix[i] = (char*)malloc(sizeof(char)*(n/2));
            next_generation_matrix = (char**)malloc(sizeof(char*)*m);
            for(int i=0;i<m;i++)next_generation_matrix[i] = (char*)malloc(sizeof(char)*(n/2));

            lseek(fd,0,SEEK_SET);
            for(int i=0;i<m;i++){
                read(fd,tmp_buffer,n);
                for(int j=0;j<n;j+=2){
                    matrix[i][j/2] = tmp_buffer[j];
                }
            }
            close(fd);
            n/=2;
            generation_count = 0;
            pid = (pid_t*)malloc(sizeof(pid_t)*process_number);     //프로세스 수 만큼 pid 할당

            if(m % process_number) handle_number = m / process_number + 1;
            else handle_number = m / process_number;
                        //행렬의 행의 수가 프로세스 개수로 나뉘어 지면 몫을, 안나누어 떨어지면 몫+1 만큼 실행

            while(generation_number--){
                if(generation_count){       //1세대 이상이면 기록된 파일 읽어오기
                    sprintf(generation_filename,"%s%d%s","gen_",generation_count,".matrix");
                    fd_old_generation = open(generation_filename,O_RDONLY);
                    for(int i=0;i<m;i++){
                        char get_buffer[n*2];
                        read(fd_old_generation,get_buffer,sizeof(get_buffer));  //파일에 기록되어 있는 한 행을 버퍼에 읽어옴
                        for(int j=0;j<n*2;j++){
                            if(j%2==0)matrix[i][j/2] = get_buffer[j];   //matrix에는 ' '와 '\n'을 제외한 값을 입력
                        }
                    }
                    close(fd_old_generation);
                }

                sprintf(generation_filename,"%s%d%s","gen_",++generation_count,".matrix");
                fd_generation = open(generation_filename,O_WRONLY|O_CREAT|O_TRUNC,0777);
                close(fd_generation);               //진행할 세대 파일 생성
                int* tmp_fd = (int*)malloc(sizeof(int)*process_number);    
                                    //프로세스 별로 기록에 사용할 파일디스크립터를 저장하도록 할당
                for(int i=0;i<process_number;i++){
                    if((pid[i] = fork()) < 0){
                        fprintf(stderr,"fork error\n");
                        exit(1);
                    }
                    else if(pid[i] == 0){           //자식프로세스인 경우
                        tmp_fd[i] = open(generation_filename,O_WRONLY);  
                                            //파일디스크립터 할당 및 파일 오픈
                        for(int j=0;j<handle_number;j++){
                            int index = i*handle_number + j;
                            if(index == m)break;
                            lseek(tmp_fd[i],index*2*n,SEEK_SET);    //파일에서 현재 처리하는 행의 위치로 이동
                            get_next_generation_matrix(matrix,next_generation_matrix,index,m-1,n-1);
                            char tmp_write[2*n];
                            for(int k=0;k<n;k++){
                                tmp_write[2*k] = next_generation_matrix[index][k];
                                if(k == n-1)tmp_write[2*k+1] = '\n';
                                else
                                    tmp_write[2*k+1] = ' ';
                            }
                            write(tmp_fd[i],tmp_write,sizeof(tmp_write));
                        }
                        close(tmp_fd[i]);
                        _exit(0);            //일을 다 처리한 후 자식프로세스 종료
                    }
                    else                      //부모프로세스인 경우
                        while(wait((int*)0) != -1);                
                }
                for(int i=0;i<process_number;i++)printf("pid[%d] : %d\n",i,pid[i]);
            }
            rename(generation_filename,"output.matrix");    //마지막세대는 output.matrix로 변경
            gettimeofday(&end,NULL);
            printf("%f ms\n",((double)end.tv_sec-start.tv_sec)*1000+((double)end.tv_usec-start.tv_usec)/1000);  //시간 및 프로세스id 출력
            break;

            case 4:
            printf("=========input the number of thread==========\n");
            scanf("%d",&thread_number);getchar();
            gettimeofday(&start,NULL);

            fd = open(argv[1],O_RDONLY);
            read(fd,tmp_buffer,20010);
            for(int i=0;i<20010;i++)
                if(tmp_buffer[i] == '\n'){
                    n=i+1;
                    break;
                }
            lseek(fd,0,SEEK_SET);

            if(!m)                      //맨 처음에만 m값을 정해야 함
                while(1){
                    if((tmp = read(fd,tmp_buffer,n)) == 0)break;
                    m++;    
                }

            matrix = (char**)malloc(sizeof(char*)*m);
            for(int i=0;i<m;i++)matrix[i] = (char*)malloc(sizeof(char)*(n/2));
            next_generation_matrix = (char**)malloc(sizeof(char*)*m);
            for(int i=0;i<m;i++)next_generation_matrix[i] = (char*)malloc(sizeof(char)*(n/2));

            lseek(fd,0,SEEK_SET);
            for(int i=0;i<m;i++){
                read(fd,tmp_buffer,n);
                for(int j=0;j<n;j+=2){
                    matrix[i][j/2] = tmp_buffer[j];
                }
            }
            close(fd);
            n/=2;
            generation_count = 0;

            pthread_t* pthread = (pthread_t*)malloc(sizeof(pthread_t)*thread_number);
            thread_data* thread_data_array = (thread_data*)malloc(sizeof(thread_data)*thread_number);

            if(m % thread_number) handle_number = m / thread_number + 1;
            else handle_number = m / thread_number;

             while(generation_number--){
                if(generation_count){       //1세대 이상이면 기록된 파일 읽어오기
                    sprintf(generation_filename,"%s%d%s","gen_",generation_count,".matrix");
                    fd_old_generation = open(generation_filename,O_RDONLY);
                    for(int i=0;i<m;i++){
                        char get_buffer[n*2];
                        read(fd_old_generation,get_buffer,sizeof(get_buffer));
                        for(int j=0;j<n*2;j++){
                            if(j%2==0)matrix[i][j/2] = get_buffer[j];
                        }
                    }
                    close(fd_old_generation);
                }

                sprintf(generation_filename,"%s%d%s","gen_",++generation_count,".matrix");
                fd_generation = open(generation_filename,O_WRONLY|O_CREAT|O_TRUNC,0777);
                close(fd_generation);
                int* tmp_fd = (int*)malloc(sizeof(int)*thread_number);
                for(int i=0;i<thread_number;i++){
                    thread_data_array[i].matrix = matrix;
                    thread_data_array[i].next_generation_matrix = next_generation_matrix;
                    thread_data_array[i].filename = generation_filename;
                    thread_data_array[i].fd = &tmp_fd[i];
                    thread_data_array[i].m = m;
                    thread_data_array[i].n = n;
                    thread_data_array[i].myindex = i;
                    thread_data_array[i].handle_number = handle_number;
                }

                for(int i=0;i<thread_number;i++)
                    pthread_create(&pthread[i],NULL,thread_func,(void*)&thread_data_array[i]);
                int tmp = n;
                int status;
                for(int i=0;i<thread_number;i++)
                    pthread_join(pthread[i],(void*)&status);
                n = tmp;
                for(int i=0;i<thread_number;i++)printf("pthread[%d] : %ld\n",i,pthread[i]);
            }
            rename(generation_filename,"output.matrix");
            gettimeofday(&end,NULL);
            
            printf("%f ms\n",(double)(end.tv_sec-start.tv_sec)*1000+(double)(end.tv_usec-start.tv_usec)/1000);
            break;
        }    
    }
    puts("=================exit program=================");
    return 0;
}

void *thread_func(void* arg){
    thread_data *data = (thread_data*)arg;
    *(data -> fd) = open(data->filename,O_WRONLY);
    for(int i=0;i<data->handle_number;i++){
        int index = data->myindex * data->handle_number + i;
        if(index >= data->m)break;
        lseek(*(data->fd),index*2*(data->n),SEEK_SET);
        get_next_generation_matrix(data->matrix,data->next_generation_matrix,index,(data)->m -1, (data->n) -1);
        char tmp_write[2*(data->n)];
        for(int j=0;j<data->n;j++){
            tmp_write[2*j] = data->next_generation_matrix[index][j];
            if(j == (data->n)-1)tmp_write[2*j+1] = '\n';
            else
                tmp_write[2*j+1] = ' ';
        }
        write(*(data->fd),tmp_write,sizeof(tmp_write));
    }
    close(*(data->fd));
    pthread_exit(NULL);
}

void get_next_generation_matrix(char** matrix, char** next_generation,int i, int horizontal, int vertical){
    for(int j=0;j<vertical+1;j++){
        int tmp = living_cell_count(matrix,i,j,horizontal,vertical);
        if(matrix[i][j] == '0'){
            if(tmp == 4)
                next_generation[i][j] = '1';
            else
                next_generation[i][j] = '0';
        }
        else{
            if(tmp<=2 || tmp>=7)
                next_generation[i][j] = '0';
            else
                next_generation[i][j] = '1';
        }
    }
}

int living_cell_count(char** matrix,int i,int j,int horizontal,int vertical){     //세포 주변의 살아있는 세포 개수를 세는 함수
    int ret = 0;
    if(i==0){
        if(j == 0){
            if(matrix[0][1]=='1')ret++;
            if(matrix[1][0]=='1')ret++;
            if(matrix[1][1]=='1')ret++;
        }
        else if(j == vertical){
            if(matrix[0][vertical-1]=='1')ret++;
            if(matrix[1][vertical]=='1')ret++;
            if(matrix[1][vertical-1]=='1')ret++;
        }
        else{
            if(matrix[i][j-1]=='1')ret++;
            if(matrix[i][j+1]=='1')ret++;
            if(matrix[i+1][j-1]=='1')ret++;
            if(matrix[i+1][j]=='1')ret++;
            if(matrix[i+1][j+1]=='1')ret++;
        }
    }
    else if(i == horizontal){
        if(j == 0){
            if(matrix[horizontal-1][0]=='1')ret++;
            if(matrix[horizontal-1][1]=='1')ret++;
            if(matrix[horizontal][1]=='1')ret++;
        }
        else if(j == vertical){
            if(matrix[horizontal-1][vertical]=='1')ret++;
            if(matrix[horizontal-1][vertical-1]=='1')ret++;
            if(matrix[horizontal][vertical-1]=='1')ret++;
        }
        else{
            if(matrix[i][j-1]=='1')ret++;
            if(matrix[i][j+1]=='1')ret++;
            if(matrix[i-1][j-1]=='1')ret++;
            if(matrix[i-1][j]=='1')ret++;
            if(matrix[i-1][j+1]=='1')ret++;
        }
    }
    else{
        if(j == 0){
            if(matrix[i-1][0]=='1')ret++;
            if(matrix[i+1][1]=='1')ret++;
            if(matrix[i-1][1]=='1')ret++;
            if(matrix[i][1]=='1')ret++;
            if(matrix[i+1][0]=='1')ret++;
        }
        else if(j == vertical){
            if(matrix[i-1][vertical]=='1')ret++;
            if(matrix[i+1][vertical]=='1')ret++;
            if(matrix[i-1][vertical-1]=='1')ret++;
            if(matrix[i][vertical-1]=='1')ret++;
            if(matrix[i+1][vertical-1]=='1')ret++;
        }
        else{
            if(matrix[i][j-1]=='1')ret++;
            if(matrix[i-1][j-1]=='1')ret++;
            if(matrix[i+1][j-1]=='1')ret++;
            if(matrix[i-1][j]=='1')ret++;
            if(matrix[i+1][j]=='1')ret++;
             if(matrix[i-1][j+1]=='1')ret++;
            if(matrix[i][j+1]=='1')ret++;
            if(matrix[i+1][j+1]=='1')ret++;
        }
    }
    return ret;
}