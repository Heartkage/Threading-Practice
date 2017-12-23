#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<stdbool.h>
#include<time.h>
#include<err.h>
#include<string.h>

#define NUM_OF_CUSTOMERS 5
#define NUM_OF_RESOURCES 3

//functions
void initial();
void *threadRunner(void *arg);
bool less_or_equal(int x[], int y[]);
bool safety();
int requestResources(int customerNum, int request[]);
int releaseResources(int customerNum, int release[]);

//Variables
int finish_count = 0;
int available[NUM_OF_RESOURCES];
int maximum[NUM_OF_CUSTOMERS][NUM_OF_RESOURCES];
int allocation[NUM_OF_CUSTOMERS][NUM_OF_RESOURCES];
int need[NUM_OF_CUSTOMERS][NUM_OF_RESOURCES];

pthread_mutex_t mutex;

int main(int argc, char *argv[]){

    if(argc != 4)
        err(1, "[Usage]: ./program resourceA resourceB resourceC\n");
    
    srand((unsigned)time(NULL));    
    
    int i, customer[NUM_OF_CUSTOMERS];
    pthread_t tid[NUM_OF_CUSTOMERS];
    pthread_mutex_init(&mutex, NULL);

    //initialization
    for(i = 0; i < NUM_OF_RESOURCES; i++){
        available[i] = atoi(argv[i+1]);
        printf("[Init] resource%d <%d> assigned\n", i, available[i]);
    }
    initial();

    //Thread Creation
    for(i = 0; i < NUM_OF_CUSTOMERS; i++){
        customer[i] = i;
        pthread_create(&tid[i], NULL, threadRunner, (void *)&customer[i]);
    }

    //Thread Join
    for(i = 0; i < NUM_OF_CUSTOMERS; i++)
        pthread_join(tid[i], NULL);
    

    if(finish_count == NUM_OF_CUSTOMERS)
        printf("[Completed] All customers have completed\n");
    else
        err(2, "[Error] Only %d customers have completed\n", finish_count);
    
    return 0;
}

void initial(){
    int i, j;
    for(i = 0; i < NUM_OF_CUSTOMERS; i++){
        for(j = 0; j < NUM_OF_RESOURCES; j++){
            maximum[i][j] = rand()%(available[j]);
            allocation[i][j] = 0;
            need[i][j] = maximum[i][j] - allocation[i][j];
        }
    }

    return;
}

void *threadRunner(void *arg){
    int current_customer = (*(int*)arg);

    while(1){
        





    }
    
    finish_count++;
    pthread_exit(0);
}

//x < y
bool less_or_equal(int x[], int y[]){
    int i;
    for(i = 0; i < NUM_OF_RESOURCES; i++){
        if(x[i] > y[i])
            return false;
    }
    return true;
}

bool safety(){
    bool finish[NUM_OF_CUSTOMERS];
    int work[NUM_OF_RESOURCES];

    int i;
    for(i = 0; i < NUM_OF_CUSTOMERS; i++)
        finish[i] = false;
    for(i = 0; i < NUM_OF_RESOURCES; i++)
        work[i] = available[i];

    while(1){
        bool check = false;
        int count = 0;

        for(i = 0; i < NUM_OF_CUSTOMERS; i++){
            if(finish[i] == false){
                if(less_or_equal(need[i], work)){
                    check = true;

                    work[0] += need[i][0];
                    work[1] += need[i][1];
                    work[2] += need[i][2];
                    finish[i] = true;
                }
            }
            else
                count++;
        }

        if(check == false){
            if(count == NUM_OF_CUSTOMERS)
                return true;
            else
                return false;
        }
    }
}


/*
 3 = succeeds & all customers finish,
 2 = succeeds & current customers finish,
 1 = succeeds,
 0 = no request,
 -1 = fails, request exceeds need,
 -2 = fails, request exceeds available,
 -3 = fails, state is unsafe
 */ 
int requestResources(int customerNum, int request[]){
    
    if(!less_or_equal(request, need[customerNum]))
        return -1;
    else if(!less_or_equal(request, available))
        return -2;
    else{
        int i;
        for(i = 0; i < NUM_OF_RESOURCES; i++){
            available[i] = available[i] - request[i];
            allocation[customerNum][i] = allocation[customerNum][i] + request[i];
            need[customerNum][i] = need[customerNum][i] - request[i];
        }

        if((need[customerNum][0]==0) && (need[customerNum][1]==0) && (need[customerNum][2]==0)){
            for(i = 0; i < NUM_OF_RESOURCES; i++){
                available[i] = available[i] + allocation[customerNum][i];
                allocation[customerNum][i] = 0;
            }   

            if(finish_count == (NUM_OF_CUSTOMERS-1))
                return 3;
            else
                return 2;
        }
        else if((request[0]==0) && (request[1]==0) && (request[2]==0))
            return 0;
        else{
            bool check = safety();
            if(check)
                return 1;
            else{
                int i;
                for(i = 0; i < NUM_OF_RESOURCES; i++){
                    available[i] = available[i] + request[i];
                    allocation[customerNum][i] = allocation[customerNum][i] - request[i];
                    need[customerNum][i] = need[customerNum][i] + request[i];       
                }
                return -3;
            }
        }
    }
}

/*
    1 = succeeds
    2 = no release
    -1 = fails, exceeds allocation
*/
int releaseResources(int customerNum, int release[]){

    if(!less_or_equal(release, allocation[customerNum]))
        return -1;
    else if((release[0]==0) && (release[1]==0) && (release[2]==0))
        return 0;
    else{
        int i;
        for(i = 0; i < NUM_OF_RESOURCES; i++){
            available[i] = available[i] + release[i];
            allocation[customerNum][i] = allocation[customerNum][i] - release[i];
            need[customerNum][i] = need[customerNum][i] + release[i];
        }
        return 1;
    }
}
