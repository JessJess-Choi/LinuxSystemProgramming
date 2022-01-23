#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<dirent.h>
#include<pwd.h>
#include<grp.h>
#include<stdlib.h>
#include<time.h>
#include<string.h>
#include<math.h>

char type(mode_t);
char* perm(mode_t);
void myls(int,char**);
void myls_minus_l_option(int,char**);
void myls_minus_i_option(int,char**);
void myls_minus_t_option(int,char**);
void myls_minus_r_option(int,char**);
void set_minus_l_option(char*,struct stat*);
void set_minus_i_option(char*,struct stat*);
void set_minus_t_option(char*,struct stat*);
void set_minus_r_option(char*,struct stat*);

DIR* dp;
char* dir;
struct stat st;
struct dirent *d;
char path[1024];
char* arr[1024];					//읽어온 디렉토리명을 저장
char* arr_l[1024];					//출력하기위한 출력값을 저장
int rewrite_time[1024];				//파일수정시간을 저장함
int arr_index;						//저장하는 배열의 길이
int sum_for_minus_l;
int check_for_option;				//옵션이 여러개 있는지 확인

int main(int argc,char* argv[]){
	int ch = 1,option[1024],option_index=0;
	if(argc == 1){
		argv[1] = (char*)malloc(strlen(".")*sizeof(char));
		strcpy(argv[1],".");
		myls(argc,argv);
		for(int i=0;i<arr_index;i++)
			printf("%s\n",arr[i]);
		puts("");
	}										//단순 ls

	else
		for(int i=0;i<argc;i++)
			if(strstr(argv[i],"-")){
				ch = 0;
				option[option_index++] = i;
			}								//옵션이 있는 경우 그 index 값을 저장
	if(ch){
		while(argc > 1){
			arr_index=0;
			myls(argc,argv);
			for(int i=0;i<arr_index;i++)
				printf("%s\n",arr[i]);
			puts("");
			argc--;
			for(int i=0;i<argc;i++){
				argv[i] = argv[i+1];
			}
		}
	}										//옵션이 없는 경우 ls 수행

	else{
		char* tmp = argv[1];
		argv[1] = argv[option[0]];
		argv[option[0]] = tmp;
		int tmp_index = 1;
		for(int i=2;i<argc;i++)
			for(int j=i;j<argc;j++)
				if((strstr(argv[i],"-") == NULL) && (strstr(argv[j],"-") != NULL)){
					char* tmp = argv[i];
					argv[i] = argv[j];
					argv[j] = tmp;
					option[tmp_index++] = i;
				}
		if(option_index > 1){
			for(int i=1;i<option_index;i++){
				char tmp1[1024];
				strcpy(tmp1,argv[1]);
				if(!strstr(argv[1],tmp1)){
					strcat(tmp1,argv[option[i]]+1);
					argv[1] = (char*)malloc(sizeof(tmp1));
					strcpy(argv[1],tmp1);
				}
				argc--;
				for(int j=option[i];j<argc;j++){
					argv[j] = argv[j+1];
				}
			}
		}
								//옵션이 다른 위치에 있는 경우 argv[1]로 옮김


		for(int i=2;i<argc;i++)
			for(int j=2;j<i;j++)
				if(strcmp(argv[i],argv[j]) < 0){
					char* tmp = argv[i];
					argv[i] = argv[j];
					argv[j] = tmp;
				}							//옵션 값을 옮긴 후 argv[0],argv[1]을 제외한 argv값들을 정렬

		while(1){
			arr_index = 0;
			check_for_option = 0;

			if(strstr(argv[1],"l") && argv[1][0] == '-'){
				myls_minus_l_option(argc,argv);
				check_for_option = 1;
			}
			if(strstr(argv[1],"i") && argv[1][0] == '-'){
				myls_minus_i_option(argc,argv);
				check_for_option = 1;
			}
			if(strstr(argv[1],"t") && argv[1][0] == '-'){
				myls_minus_t_option(argc,argv);
				check_for_option = 1;
			}
			if(strstr(argv[1],"r") && argv[1][0] == '-'){
				myls_minus_r_option(argc,argv);
				check_for_option = 1;
			}								//옵션 수행
			for(int i=0;i<arr_index;i++){
				printf("%s",arr_l[i]);
			}										//결과값 출력
			puts("\n");
			argc--;
			if(argc <= 2)break;
			for(int i=2;i<argc;i++)
				argv[i] = argv[i+1];			//인자들을 옮겨 줌
		}
	}

	closedir(dp);
	exit(0);
}

void myls(int argc,char* argv[]){
	int check = 1;
	if(argc == 1)dir = ".";
	else dir = argv[1];						//디렉토리 경로 설정
	if((dp = opendir(dir)) == NULL){		//argv[1]이 옵션일 수도, 파일명일 수도, 없는 파일명일 수도 있다
		dir = ".";
		if((dp = opendir(dir)) == NULL){
			puts("error");
			exit(1);
		}
		while((d = readdir(dp)) != NULL){
			sprintf(path,"%s/%s",dir,d->d_name);
			if(!lstat(path,&st) < 0){
				puts("error");
				exit(1);
			}
			if(strcmp(d->d_name,".") && strcmp(d->d_name,".."))
				arr[arr_index++] = d->d_name;
		}											//디렉토리에 있는 파일명 읽어와 저장
		for(int i=0;i<arr_index;i++)
			if(!strcmp(arr[i],argv[1])){
				char* tmp = arr[0];
				arr[0] = arr[i];
				arr[i] = tmp;
				arr_index = 1;
				check = 0;
				break;
			}											//파일이 있는 경우 출력
		if(check)
			printf("NO FILES : %s\n",argv[1]);			//파일이 없는 경우 에러처리
	}
	else{
		while((d = readdir(dp)) != NULL){
			sprintf(path,"%s/%s",dir,d->d_name);
			if(!lstat(path,&st) < 0){
				puts("error");
				exit(1);
			}
			if(strcmp(d->d_name,".") && strcmp(d->d_name,".."))
				arr[arr_index++] = d->d_name;
		}												//디렉토리에 있는 파일명 읽어와 저장
		for(int i=0;i<arr_index;i++)
				for(int j=0;j<i;j++)
					if(strcmp(arr[i],arr[j]) < 0){
						char change[1024];
						strcpy(change,arr[i]);
						strcpy(arr[i],arr[j]);
						strcpy(arr[j],change);
					}									//이름순 정렬
	}
}


void myls_minus_l_option(int argc,char* argv[]){
	int check = 1;
	if(argc == 2)dir = ".";
	else
		dir = argv[2];
	if((dp = opendir(dir)) == NULL){												//argv[2]가 파일명이면 NULL
		dir = ".";
		if((dp = opendir(dir)) == NULL){
			puts("error");
			exit(1);
		}																			
		while((d = readdir(dp)) != NULL){
			sprintf(path,"%s/%s",dir,d->d_name);
			if(!lstat(path,&st) < 0){
				puts("error");
				exit(1);
			}
			if(!strcmp(d->d_name,argv[2])){												//파일이 있는 경우
				char tmp[1024];
				arr[arr_index] = d->d_name;
				sprintf(tmp,"%c%s %3ld %s %s %9ld %.12s %s\n",type(st.st_mode),perm(st.st_mode),st.st_nlink,getpwuid(st.st_uid)->pw_name,getgrgid(st.st_gid)->gr_name,st.st_size,ctime(&st.st_mtime)+4,d->d_name);
				arr_l[arr_index] = (char*)malloc(sizeof(tmp));
				strcpy(arr_l[arr_index],tmp);
				rewrite_time[arr_index] = st.st_mtime;
				arr_index++;
			}
		}
		for(int i=0;i<arr_index;i++)
			if(!strcmp(arr[i],argv[2])){
				check = 0;
				break;
			}
		if(check)
			printf("NO FILES : %s\n",argv[2]);									//파일이 없는 경우
	}
	else{
		while((d = readdir(dp)) != NULL){
			sprintf(path,"%s/%s",dir,d->d_name);
			if(!lstat(path,&st) < 0){
				puts("error");
				exit(1);
			}
			set_minus_l_option(d->d_name,&st);
		}
	}
	for(int i=0;i<arr_index;i++)
		for(int j=0;j<i;j++)
			if(strcmp(arr[i],arr[j]) < 0){
				char change[1024],change1[1024];
				strcpy(change,arr[i]);
				strcpy(arr[i],arr[j]);
				strcpy(arr[j],change);
				
				strcpy(change1,arr_l[i]);
				strcpy(arr_l[i],arr_l[j]);
				strcpy(arr_l[j],change1);

				int tmp = rewrite_time[i];
				rewrite_time[i] = rewrite_time[j];
				rewrite_time[j] = tmp;
			}	
}

void myls_minus_i_option(int argc,char* argv[]){
	int check = 1;
	char tmp[1024];
	if(argc == 2)dir = ".";
	else
		dir = argv[2];
	if((dp = opendir(dir)) == NULL){
		dir = ".";
		if((dp = opendir(dir)) == NULL){
			puts("error");
			exit(1);
		}
		while((d = readdir(dp)) != NULL){
			sprintf(path,"%s/%s",dir,d->d_name);
			if(!lstat(path,&st) < 0){
				puts("error");
				exit(1);
			}
			if(!strcmp(d->d_name,argv[2])){
				if(check_for_option){													//다른 옵션을 이미 실행한 경우
					for(int i=0;i<arr_index;i++)
						if(strstr(arr[i],d->d_name)){
							sprintf(tmp,"%ld %s",st.st_ino,arr_l[i]);
							arr_l[i] = (char*)malloc(sizeof(tmp));
							strcpy(arr_l[i],tmp);
							break;
						}
				}
				else{																	//-i옵션만 진행하는 경우
					arr[arr_index] = d->d_name;
					sprintf(tmp,"%ld %s",st.st_ino,d->d_name);
					arr_l[arr_index] = (char*)malloc(sizeof(tmp));
					strcpy(arr_l[arr_index],tmp);
					rewrite_time[arr_index] = st.st_mtime;
					arr_index++;
				}
			}
		}
		for(int i=0;i<arr_index;i++)
			if(!strcmp(arr[i],argv[2])){
				check = 0;
				break;
			}
		if(check)
			printf("NO FILES : %s\n",argv[2]);
	}
	else{
		while((d = readdir(dp)) != NULL){
			sprintf(path,"%s/%s",dir,d->d_name);
			if(!lstat(path,&st) < 0){
				puts("error");
				exit(1);
			}
			set_minus_i_option(d->d_name,&st);
		}
	}
	for(int i=0;i<arr_index;i++)
		for(int j=0;j<i;j++)
			if(strcmp(arr[i],arr[j]) < 0){
				char change[1024],change1[1024];
				strcpy(change,arr[i]);
				strcpy(arr[i],arr[j]);
				strcpy(arr[j],change);
				
				strcpy(change1,arr_l[i]);
				strcpy(arr_l[i],arr_l[j]);
				strcpy(arr_l[j],change1);

				int tmp = rewrite_time[i];
				rewrite_time[i] = rewrite_time[j];
				rewrite_time[j] = tmp;
			}
}

void myls_minus_t_option(int argc,char* argv[]){
	int check = 1;
	char tmp[1024];
	if(argc == 2)dir = ".";
	else
		dir = argv[2];
	if((dp = opendir(dir)) == NULL){
		dir = ".";
		if((dp = opendir(dir)) == NULL){
			puts("error");
			exit(1);
		}
		while((d = readdir(dp)) != NULL){
			sprintf(path,"%s/%s",dir,d->d_name);
			if(!lstat(path,&st) < 0){
				puts("error");
				exit(1);
			}
			if(!strcmp(d->d_name,argv[2])){
				if(!check_for_option){
					arr[arr_index] = d->d_name;
					sprintf(tmp,"%s\n",d->d_name);
					arr_l[arr_index] = (char*)malloc(sizeof(tmp));
					strcpy(arr_l[arr_index],tmp);
					rewrite_time[arr_index] = st.st_mtime;
					arr_index++;
				}
			}
		}
		for(int i=0;i<arr_index;i++){
			if(!strcmp(arr[i],argv[2])){
				check = 0;
				break;
			}
		}
		if(check)
			printf("NO FILES : %s\n",argv[2]);
	}
	else{
		while((d = readdir(dp)) !=NULL){
			sprintf(path,"%s/%s",dir,d->d_name);
			if(!lstat(path,&st) < 0){
				puts("error");
				exit(1);
			}
			set_minus_t_option(d->d_name,&st);
		}
	}
	for(int i=0;i<arr_index;i++)						//수정한 시간순 정렬
			for(int j=0;j<i;j++)
				if(rewrite_time[i] > rewrite_time[j]){
					char tmp[1024],tmp1[1024];
					strcpy(tmp,arr[i]);
					strcpy(arr[i],arr[j]);
					strcpy(arr[j],tmp);

					strcpy(tmp1,arr_l[i]);
					strcpy(arr_l[i],arr_l[j]);
					strcpy(arr_l[j],tmp1);

					int tmp2 = rewrite_time[i];
					rewrite_time[i] = rewrite_time[j];
					rewrite_time[j] = tmp2;
				}
	for(int i=0;i<arr_index;i++){						//시간이 같을 경우 이름순 정렬
		for(int j=0;j<i;j++){
			if(rewrite_time[i] == rewrite_time[j]){
				if(strcmp(arr[i],arr[j]) < 0){
					char change[1024],change1[1024];
					strcpy(change,arr[i]);
					strcpy(arr[i],arr[j]);
					strcpy(arr[j],change);
					
					strcpy(change1,arr_l[i]);
					strcpy(arr_l[i],arr_l[j]);
					strcpy(arr_l[j],change1);

					int tmp = rewrite_time[i];
					rewrite_time[i] = rewrite_time[j];
					rewrite_time[j] = tmp;
				}
			}
		}
	}
}

void myls_minus_r_option(int argc,char* argv[]){
	int check = 1;
	char tmp[1024];
	if(argc == 2)dir = ".";
	else
		dir = argv[2];
	if((dp = opendir(dir)) == NULL){
		dir = ".";
		if((dp = opendir(dir)) == NULL){
			puts("error");
			exit(1);
		}
		while((d = readdir(dp)) != NULL){
			sprintf(path,"%s/%s",dir,d->d_name);
			if(!lstat(path,&st) < 0){
				puts("error");
				exit(1);
			}
			if(!strcmp(d->d_name,argv[2])){
				if(!check_for_option){
					arr[arr_index] = d->d_name;
					sprintf(tmp,"%s\n",d->d_name);
					arr_l[arr_index] = (char*)malloc(sizeof(tmp));
					strcpy(arr_l[arr_index],tmp);
					rewrite_time[arr_index] = st.st_mtime;
					arr_index++;
				}
			}
		}
		for(int i=0;i<arr_index;i++){
			if(!strcmp(arr[i],argv[2])){
				check = 0;
				break;
			}
		}
		if(check)
			printf("NO FILES : %s\n",argv[2]);
	}
	else{
		while((d = readdir(dp)) != NULL){
			sprintf(path,"%s/%s",dir,d->d_name);
			if(!lstat(path,&st) < 0){
				puts("error");
				exit(1);
			}
			set_minus_r_option(d->d_name,&st);
		}
	}
	for(int i=0;i<arr_index/2;i++){
		char* change = arr_l[i];
		arr_l[i] = arr_l[arr_index - i-1];
		arr_l[arr_index - i-1] = change;
	}
}

void set_minus_l_option(char *file,struct stat *st){
	if(strcmp(file,".") && strcmp(file,"..")){
		char tmp[1024];
		arr[arr_index] = file;
		
		sprintf(tmp,"%c%s %3ld %s %s %9ld %.12s %s\n",type(st->st_mode),perm(st->st_mode),st->st_nlink,getpwuid(st->st_uid)->pw_name,getgrgid(st->st_gid)->gr_name,st->st_size,ctime(&st->st_mtime)+4,file);
		arr_l[arr_index] = (char*)malloc(sizeof(tmp));
		rewrite_time[arr_index] = st->st_mtime;
		strcpy(arr_l[arr_index],tmp);
		arr_index++;
	}
}
void set_minus_i_option(char *file,struct stat *st){
	if(strcmp(file,".") && strcmp(file,"..")){
		char tmp[1024];

		if(check_for_option){
			for(int i=0;i<arr_index;i++)
				if(strstr(arr[i],file)){
					sprintf(tmp,"%7ld %s",st->st_ino,arr_l[i]);
					arr_l[i] = (char*)malloc(sizeof(tmp));
					strcpy(arr_l[i],tmp);
					break;
				}
		}
		else{
			arr[arr_index] = file;
			sprintf(tmp,"%7ld %s\n",st->st_ino,file);
			arr_l[arr_index] = (char*)malloc(sizeof(tmp));
			rewrite_time[arr_index] = st->st_mtime;
			strcpy(arr_l[arr_index],tmp);
			arr_index++;
		}
	}
}

void set_minus_t_option(char *file,struct stat *st){
	char tmp[1024];
	if(strcmp(file,".") && strcmp(file,"..")){
		if(!check_for_option){										//-t옵션만 있는 경우
			arr[arr_index] = file;
			sprintf(tmp,"%s\n",file);
			arr_l[arr_index] = (char*)malloc(sizeof(tmp));
			strcpy(arr_l[arr_index],tmp);
			rewrite_time[arr_index] = st->st_mtime;
			arr_index++;
		}
	}
}

void set_minus_r_option(char *file,struct stat *st){
	char tmp[1024];
		if(strcmp(file,".") && strcmp(file,"..")){
			if(!check_for_option){										//-t옵션만 있는 경우
				arr[arr_index] = file;
				sprintf(tmp,"%s\n",file);
				arr_l[arr_index] = (char*)malloc(sizeof(tmp));
				strcpy(arr_l[arr_index],tmp);
				rewrite_time[arr_index] = st->st_mtime;
				arr_index++;
			}
		}
}

char type(mode_t mode){
	if(S_ISREG(mode))return('-');
	if(S_ISDIR(mode))return('d');
	if(S_ISCHR(mode))return('c');
	if(S_ISBLK(mode))return('b');
	if(S_ISLNK(mode))return('l');
	if(S_ISFIFO(mode))return('p');
	if(S_ISSOCK(mode))return('s');
}

char* perm(mode_t mode){
	static char perms[10]="---------";
	for(int i=0;i<3;i++){
		if(mode & (S_IRUSR >> i*3))perms[i*3] = 'r';
		if(mode & (S_IWUSR >> i*3))perms[i*3+1] = 'w';
		if(mode & (S_IXUSR >> i*3))perms[i*3+2]='x';
	}
	return(perms);
}
