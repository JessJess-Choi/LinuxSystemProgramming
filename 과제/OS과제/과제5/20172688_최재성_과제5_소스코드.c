#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>

int page_frame, arr[31];
int page_reference[31];
//char page_reference[62];
FILE *fp;

void OPT();
void FIFO();
void LRU();
void SCR();
bool find(int*,int);
void queue_push(int*,int*,int);
void page_change(int*,int*,int,int);
void stack_update(int*,int,int);
void vector_update(int*,int,char*,int);
void page_change(int*,int*,int,int);
void queue_pagebit_change(int*,int*,int*,int);
void pagebit_change(int*,int*,int,int);

int main(int argc, char *argv[]){
	if(argc != 2){
		fprintf(stderr,"input arguments error\n");
		exit(1);
	}

	if((fp = fopen(argv[1],"r")) == NULL){
		fprintf(stderr,"file open error : %s\n",argv[1]);
		exit(1);
	}
	fscanf(fp,"%d\n",&page_frame);
	fgets(page_reference,62,fp);

	OPT();
	FIFO();
	LRU();
	SCR();

	fclose(fp);
	return 0;
}

void vector_update(int *vector, int vector_number,char *page_reference, int indx){
	if(vector_number < page_frame)
		vector[vector_number] = page_reference[indx] - '0';
	else{
		int max_indx = 0;	
		for(int i=0;i<indx/2;i++)
			if(arr[max_indx] < arr[i]){
				max_indx = i;
			}
		for(int i=0;i<page_frame;i++){
			if(vector[i] == page_reference[max_indx * 2] - '0'){
				vector[i] = page_reference[indx] - '0';
				arr[max_indx] = -1;
			}
		}
	}
}

void OPT(){
	int *vector, vector_number = 0, page_fault = 0;
	vector = (int*)malloc(sizeof(int) * page_frame);
	memset(vector,-1,sizeof(int) * page_frame);

	for(int i=0;i<62;i+=2){
		int cnt = 1;
		for(int j=i + 2;j<62;j+=2){
			if(!page_reference[j] || page_reference[i] == page_reference[j])
			break;
			cnt++;
		}
		arr[i / 2] = cnt;
	}

	printf("Used method : OPT\n");
	printf("page reference string : %s\n",page_reference);
	printf("	frame");
	for(int i=0;i<page_frame;i++)
		printf("	%d",i+1);
	printf("	page fault\n");
	printf("time\n");
	for(int i=0;i<62;i+=2){

		if(!page_reference[i])
			break;
		printf("%d		",(i + 2)/2);

		for(int j=0;j<i;j+=2)
			for(int k=j+2;k<=i;k+=2)
				if(page_reference[j] == page_reference[k]){
					arr[j/2] = -1;
					break;
				}
		for(int j=0;j<i/2;j++)
			if(arr[j] > 1)
				arr[j]--;

		if(find(vector,page_reference[i] - '0')){
			for(int j=0;j<page_frame;j++){
				if(vector[j] != -1)
					printf("%d",vector[j]);
				printf("	");
			}
			printf("\n");
		}
		else{
			page_fault++;
			vector_update(vector,vector_number++,page_reference,i);
			for(int j=0;j<page_frame;j++){
				if(vector[j] != -1)
					printf("%d",vector[j]);
				printf("	");
			}
			printf("F\n");
		}
	}
	printf("Number of page faults : %d times\n\n",page_fault);
}

bool find(int *object, int page_reference){
	for(int i=0;i<page_frame;i++)
		if(object[i] == page_reference)return true;
	return false;
}

void queue_push(int *queue, int *queue_number, int page_reference){
	queue[*queue_number] = page_reference;
	*queue_number = (*queue_number + 1) % page_frame;
}

void FIFO(){
	int *queue, queue_number = 0, page_fault = 0;
	queue = (int*)malloc(sizeof(int) * page_frame);
	memset(queue,-1,sizeof(int) * page_frame);

	printf("Used method : FIFO\n");
	printf("page reference string : %s\n",page_reference);
	printf("	frame");
	for(int i=0;i<page_frame;i++)
		printf("	%d",i+1);
	printf("	page fault\n");
	printf("time\n");
	for(int i=0;i<62;i+=2){
		if(!page_reference[i])
			break;
		printf("%d		",(i + 2)/2);

		if(find(queue,page_reference[i] - '0')){
			for(int j=0;j<page_frame;j++){
				if(queue[j] != -1)
					printf("%d",queue[j]);
				printf("	");
			}
			printf("\n");
		}
		else{
			page_fault++;
			queue_push(queue,&queue_number,page_reference[i] - '0');
			for(int j=0;j<page_frame;j++){
				if(queue[j] != -1)
					printf("%d",queue[j]);
				printf("	");
			}
			printf("F\n");
		}
	}
	printf("Number of page faults : %d times\n\n",page_fault);
}

void page_change(int *page, int *stack, int stack_number, int page_reference){
	if(stack_number < page_frame){
		page[stack_number] = page_reference;
	}
	else{
		for(int i=0;i<page_frame;i++)
			if(page[i] == stack[page_frame - 1]){
				page[i] = page_reference;
				break;
			}
		for(int i=page_frame - 1;i>0;i--)
			stack[i] = stack[i - 1];
		stack[0] = page_reference;
	}
}

void stack_update(int *stack, int stack_number, int page_reference){
	if(stack_number < page_frame){
		for(int i=stack_number;i>0;i--)
			stack[i] = stack[i - 1];
		stack[0] = page_reference;
	}
	else{
		if(find(stack,page_reference)){
			int indx = -1, tmp = -1;
			for(int i=0;i<page_frame;i++){
				if(stack[i] == page_reference){
					indx = i;
					break;
				}
			}
			tmp = stack[indx];
			for(int i=indx;i>0;i--)
				stack[i] = stack[i - 1];
			stack[0] = tmp;
		}
		else{
			for(int i=1;i<page_frame;i++)
				stack[i] = stack[i - 1];
			stack[0] = page_reference;
		}
	}
}

void LRU(){
	int *stack, *page, stack_number = 0, page_fault = 0;
	stack = (int*)malloc(sizeof(int) * page_frame);
	page = (int*)malloc(sizeof(int) * page_frame);
	memset(stack,-1,sizeof(int) * page_frame);
	memset(page,-1,sizeof(int) * page_frame);

	printf("Used method : LRU\n");
	printf("page reference string : %s\n",page_reference);
	printf("	frame");
	for(int i=0;i<page_frame;i++)
		printf("	%d",i+1);
	printf("	page fault\n");
	printf("time\n");
	for(int i=0;i<62;i+=2){
		if(!page_reference[i])
			break;
		printf("%d		",(i + 2)/2);

		if(find(page,page_reference[i] - '0')){
			stack_update(stack,stack_number,page_reference[i] - '0');
			for(int j=0;j<page_frame;j++){
				if(page[j] != -1)
					printf("%d",page[j]);
				printf("	");
			}
			printf("\n");
		}
		else{
			page_fault++;
			page_change(page,stack,stack_number,page_reference[i] - '0');	//페이지 frame 바꿈
			stack_update(stack,stack_number++,page_reference[i] - '0');
			for(int j=0;j<page_frame;j++){
				if(page[j] != -1)
					printf("%d",page[j]);
				printf("	");
			}
			printf("F\n");
		}
	}
	printf("Number of page faults : %d times\n\n",page_fault);
}

void queue_pagebit_change(int *queue, int *page_bit, int *queue_number, int page_reference){
	if(*queue_number < page_frame){
		queue[*queue_number] = page_reference;
		page_bit[*queue_number] = 0;
	}
	else{
		while(1){
			if(page_bit[*queue_number % page_frame]){
				page_bit[*queue_number % page_frame] = 0;
			}
			else
				break;
			*queue_number = *queue_number + 1;
		}
		queue[*queue_number % page_frame] = page_reference;
	}
}

void pagebit_change(int *queue, int *page_bit, int queue_number, int page_reference){
	int i;
	for(i=0;i<page_frame;i++)
		if(queue[i] == page_reference)
			break;
	page_bit[i] = 1;
}

void SCR(){
	int *queue, *page_bit, queue_number = 0, page_fault = 0;
	queue = (int*)malloc(sizeof(int) * page_frame);
	page_bit = (int*)malloc(sizeof(int) * page_frame);
	memset(queue,-1,sizeof(int) * page_frame);
	memset(page_bit,0,sizeof(int) * page_frame);

	printf("Used method : Second-Chance\n");
	printf("page reference string : %s\n",page_reference);
	printf("	frame");
	for(int i=0;i<page_frame;i++)
		printf("	%d",i+1);
	printf("	page fault\n");
	printf("time\n");
	for(int i=0;i<62;i+=2){
		if(!page_reference[i])
			break;
		printf("%d		",(i + 2)/2);
		if(find(queue,page_reference[i] - '0')){
			pagebit_change(queue,page_bit,queue_number,page_reference[i] - '0');
			for(int j=0;j<page_frame;j++){
				if(queue[j] != -1)
					printf("%d",queue[j]);
				printf("	");
			}
			printf("\n");
		}
		else{
			page_fault++;
			queue_pagebit_change(queue,page_bit,&queue_number,page_reference[i] - '0');
			queue_number++;
			for(int j=0;j<page_frame;j++){
				if(queue[j] != -1)
					printf("%d",queue[j]);
				printf("	");
			}
			printf("F\n");
		}
	}
	printf("Number of page faults : %d times\n\n",page_fault);
}