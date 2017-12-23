#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<stdbool.h>
#include<time.h>
#include<err.h>
#include<string.h>

#define NUM_OF_CUSTOMERS 5
#define NUM_OF_RESOURCES 3


void *threadRunner(void *arg);

int available[NUM_OF_RESOURCES];
int maximum[NUM_OF_CUSTOMERS][NUM_OF_RESOURCES];
int allocation[NUM_OF_CUSTOMERS][NUM_OF_RESOURCES];
int need[NUM_OF_CUSTOMERS][NUM_OF_RESOURCES];

int finish_count = 0;
pthread_mutex_t mutex;

int main(int argc, char *argv[]){

    if(argc != 4)
        err(1, "[Usage]: ./program resourceA resourceB resourceC\n");
    
    srand((unsigned)time(NULL));    
    
    int i, customer[NUM_OF_CUSTOMERS];
    pthread_t tid[NUM_OF_CUSTOMERS];
    pthread_mutex_init(&mutex, NULL);

    //initial
    for(i = 0; i < NUM_OF_RESOURCES; i++){
        available[i] = atoi(argv[i+1]);
        printf("[Init] resource%d <%d> assigned\n", i, available[i]);
    }

    for(i = 0; i < NUM_OF_CUSTOMERS; i++){
        customer[i] = i;
        pthread_create(&tid[i], NULL, threadRunner, (void *)&customer[i]);
    }

    for(i = 0; i < NUM_OF_CUSTOMERS; i++)
        pthread_join(tid[i], NULL);
    

    if(finish_count == NUM_OF_CUSTOMERS)
        printf("[Completed] All customers have completed\n");
    else
        err(2, "[Error] Only %d customers have completed\n", finish_count);
    

    return 0;
}

void *threadRunner(void *arg){
    int current_customer = (*(int*)arg);






    
    finish_count++;
    pthread_exit(0);
}
