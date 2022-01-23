#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<pthread.h>
#include<signal.h>
#include<stdbool.h>
#include<unistd.h>
#include<sys/wait.h>

void *thread_function1(void *);
void *thread_function2(void *);
void *thread_function3(void *);
void *thread_function4(void *);
void printMovingCar();
void printWaitingList();
void addSet(int);
bool isEmpty();
void removeItem(int);

int tick = 1, input, *start_list, indx = 1, P1, P2, P3, P4, passed;

typedef struct set{
	int arr[15];
	int howMany;
}set;

set s;

pthread_t tid1,tid2,tid3,tid4;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex4 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond3 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond4 = PTHREAD_COND_INITIALIZER;

int main(){
	printf("Total number of vehicles : ");
	scanf("%d",&input);
	start_list = (int*)malloc(sizeof(int)*input);
	srand(time(NULL));
	
	for(int i=0;i<input;i++)
		start_list[i] = rand() % 4 + 1;
	printf("Start point : ");
	for(int i=0;i<input;i++)printf("%d ",start_list[i]);
	printf("\n");
	
	pthread_create(&tid1,NULL,thread_function1,NULL);
	pthread_create(&tid2,NULL,thread_function2,NULL);
	pthread_create(&tid3,NULL,thread_function3,NULL);
	pthread_create(&tid4,NULL,thread_function4,NULL);

	usleep(1000);

	int item = start_list[0];

	if(item == 1)pthread_cond_signal(&cond1);
	else if(item == 2)pthread_cond_signal(&cond2);
	else if(item == 3)pthread_cond_signal(&cond3);
	else if(item == 4)pthread_cond_signal(&cond4);

	pthread_join(tid1,(void*)NULL);
	pthread_join(tid2,(void*)NULL);
	pthread_join(tid3,(void*)NULL);
	pthread_join(tid4,(void*)NULL);

	pthread_mutex_destroy(&mutex1);
	pthread_mutex_destroy(&mutex2);
	pthread_mutex_destroy(&mutex3);
	pthread_mutex_destroy(&mutex4);

	pthread_cond_destroy(&cond1);
	pthread_cond_destroy(&cond2);
	pthread_cond_destroy(&cond3);
	pthread_cond_destroy(&cond4);

	printf("Number of vehicles passed from each start point\n");
	printf("P1 : %d\n",P1);
	printf("P2 : %d\n",P2);
	printf("P3 : %d\n",P3);
	printf("P4 : %d\n",P4);
	printf("Total time : %d ticks\n",tick);
	return 0;
}

void printMovingCar(){
	printf("tick : %d\n=============\nPassed Vehicle\nCar ",tick);
	if(passed)
		printf("%d",passed);
	printf("\n");
	passed = 0;
}

void printWaitingList(){
	printf("Waiting Vehicle\nCar ");
	if(tick > 1)
		for(int i=0;i<s.howMany;i++)
			printf("%d ",s.arr[i]);
	printf("\n===========\n");
}

void addSet(int add){
	if(indx < input){
		s.arr[s.howMany++] = add;
		indx++;
	}
}

int getItem(){
	int ret, ret_index;
	if(s.howMany){
		ret_index = rand() % s.howMany;
		ret = s.arr[ret_index];
		for(int i=ret_index;i<s.howMany-1;i++)
			s.arr[i] = s.arr[i + 1];
		s.howMany--;
	}
	return ret;
}

bool isEmpty(){
	return s.howMany <= 0;
}

bool find(int n){
	for(int i=0;i<s.howMany;i++)
		if(s.arr[i] == n)
			return true;
	return false;
}

void removeItem(int n){
	for(int i=0;i<s.howMany;i++)
		if(s.arr[i] == n){
			for(int j=i;j<s.howMany-1;j++)
				s.arr[j] = s.arr[j+1];
			break;
		}
	if(s.howMany > 0)
		s.howMany--;
}

void *thread_function1(void *args){
	while(true){
		pthread_mutex_lock(&mutex1);
		pthread_cond_wait(&cond1,&mutex1);
		
		if(P1 + P2 + P3 + P4 == input){
			pthread_mutex_unlock(&mutex1);
			pthread_exit(NULL);
		}	

		GO:
		printMovingCar();
		printWaitingList();

		usleep(1000);
		tick++;
		P1++;

		addSet(start_list[indx]);
		passed = 1;

		if(find(3)){
			removeItem(3);
			pthread_cond_signal(&cond3);
		}
		else{
			printMovingCar();
			printWaitingList();
			usleep(1000);
			tick++;
			addSet(start_list[indx]);
			int item = getItem();
			if(P1 + P2 + P3 + P4 == input){
				printMovingCar();
				printWaitingList();
				pthread_mutex_unlock(&mutex1);
				pthread_cond_signal(&cond2);
				pthread_cond_signal(&cond3);
				pthread_cond_signal(&cond4);
				pthread_exit(NULL);
			}

			if(item == 1)goto GO;
			else if(item == 2)pthread_cond_signal(&cond2);
			else if(item == 3)pthread_cond_signal(&cond3);
			else if(item == 4)pthread_cond_signal(&cond4);
		}

		pthread_mutex_unlock(&mutex1);
	}
}
void *thread_function2(void *args){
	while(true){
		pthread_mutex_lock(&mutex2);
		pthread_cond_wait(&cond2,&mutex2);

		if(P1 + P2 + P3 + P4 == input ){
			pthread_mutex_unlock(&mutex2);
			pthread_exit(NULL);
		}

		GO:
		printMovingCar();
		printWaitingList();

		usleep(1000);
		tick++;
		P2++;
		addSet(start_list[indx]);
		passed = 2;

		if(find(4)){
			removeItem(4);
			pthread_cond_signal(&cond4);
		}
		else{
			printMovingCar();
			printWaitingList();
			usleep(1000);
			tick++;
			addSet(start_list[indx]);

			int item = getItem();

			if(P1 + P2 + P3 + P4 == input ){
				printMovingCar();
				printWaitingList();
				pthread_mutex_unlock(&mutex2);
				pthread_cond_signal(&cond1);
				pthread_cond_signal(&cond3);
				pthread_cond_signal(&cond4);
				pthread_exit(NULL);
			}

			if(item == 1)pthread_cond_signal(&cond1);
			else if(item == 2)goto GO;
			else if(item == 3)pthread_cond_signal(&cond3);
			else if(item == 4)pthread_cond_signal(&cond4);
		}

		pthread_mutex_unlock(&mutex2);
	}
}
void *thread_function3(void *args){
	while(true){
		pthread_mutex_lock(&mutex3);
		pthread_cond_wait(&cond3,&mutex3);

		if(P1 + P2 + P3 + P4 == input){
			pthread_mutex_unlock(&mutex3);
			pthread_exit(NULL);
		}

		GO:
		printMovingCar();
		printWaitingList();
		usleep(1000);
		tick++;
		P3++;

		addSet(start_list[indx]);
		passed = 3;

		if(find(1)){
			removeItem(1);
			pthread_cond_signal(&cond1);
		}
		else{
			printMovingCar();
			printWaitingList();
			usleep(1000);
			tick++;
			addSet(start_list[indx]);
			
			int item = getItem();

			if(P1 + P2 + P3 + P4 == input ){
				printMovingCar();
				printWaitingList();
				pthread_mutex_unlock(&mutex3);
				pthread_cond_signal(&cond1);
				pthread_cond_signal(&cond2);
				pthread_cond_signal(&cond4);
				pthread_exit(NULL);
			}

			if(item == 1)pthread_cond_signal(&cond1);
			else if(item == 2)pthread_cond_signal(&cond2);
			else if(item == 3)goto GO;
			else if(item == 4)pthread_cond_signal(&cond4);
		}

		pthread_mutex_unlock(&mutex3);

	}
}
void *thread_function4(void *args){
	while(true){
		pthread_mutex_lock(&mutex4);
		pthread_cond_wait(&cond4,&mutex4);

		if(P1 + P2 + P3 + P4 == input ){
			pthread_mutex_unlock(&mutex4);
			pthread_exit(NULL);
		}

		GO:
		printMovingCar();
		printWaitingList();
		usleep(1000);
		tick++;
		P4++;

		addSet(start_list[indx]);
		passed = 4;

		if(find(2)){
			removeItem(2);
			pthread_cond_signal(&cond2);
		}
		else{
			printMovingCar();
			printWaitingList();
			usleep(1000);
			tick++;
			addSet(start_list[indx]);

			int item = getItem();
			if(P1 + P2 + P3 + P4 == input ){
				printMovingCar();
				printWaitingList();
				pthread_mutex_unlock(&mutex4);
				pthread_cond_signal(&cond1);
				pthread_cond_signal(&cond2);
				pthread_cond_signal(&cond3);
				pthread_exit(NULL);
			}

			if(item == 1)pthread_cond_signal(&cond1);
			else if(item == 2)pthread_cond_signal(&cond2);
			else if(item == 3)pthread_cond_signal(&cond3);
			else if(item == 4)goto GO;
		}

		pthread_mutex_unlock(&mutex4);
	}
}
