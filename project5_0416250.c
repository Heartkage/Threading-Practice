#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<stdbool.h>
#include<unistd.h>
#include<time.h>
#include<err.h>
#include<string.h>
#include<sched.h>

#define NUM_OF_CUSTOMERS 5
#define NUM_OF_RESOURCES 3

//functions
void initial();
void *threadRunner(void *arg);
bool less_or_equal(int x[], int y[]);
bool safety();
int requestResources(int customerNum, int request[]);
int releaseResources(int customerNum, int release[]);
void print_info(bool, int, int);

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
            int temp = rand()%(available[j]+2);
            maximum[i][j] = (temp == 0)? temp : temp-1;
        
            allocation[i][j] = 0;
            need[i][j] = maximum[i][j] - allocation[i][j];
        }
    }
    return;
}

void *threadRunner(void *arg){
    int current_customer = (*(int*)arg);

    while(1){
        int i, value;
        int temp[NUM_OF_RESOURCES];
        
        for(i = 0; i < NUM_OF_RESOURCES; i++){
            int temp2 = rand()%(maximum[current_customer][i]+2);
            temp[i] = (temp2 == 0)? temp2 : temp2-1;
            //printf("%d ", temp[i]);
        }

        //requesting
        pthread_mutex_lock(&mutex);
        
        printf("Request %d %d %d\n", temp[0], temp[1], temp[2]);
        value = requestResources(current_customer, temp);
        print_info(true, current_customer, value);

        //sched_yield();
        pthread_mutex_unlock(&mutex);


        
        sleep(1);
        //sched_yield();
        if((value==2) || (value==3))
            break;


        for(i = 0; i < NUM_OF_RESOURCES; i++){
            int temp2 = rand()%(maximum[current_customer][i]+2);
            temp[i] = (temp2 == 0)? temp2 : temp2-1;
        }
            
        //releasing
        pthread_mutex_lock(&mutex);

        printf("Release %d %d %d\n", temp[0], temp[1], temp[2]);
        value = releaseResources(current_customer, temp);
        print_info(false, current_customer, value);

        pthread_mutex_unlock(&mutex);
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

void print_info(bool first, int customer, int value){
    int i;
    //print request/release code
    if(first){
        switch(value){
        case 3:{
                   printf("Request Code 3: customer %d's request succeeds & all customers finish\n", customer);
                   break;
               }
        case 2:{
                   printf("Request Code 2: customer %d's request succeeds & finishes\n", customer);
                   break;
               }
        case 1:{
                   printf("Request Code 1: customer %d's request succeeds\n", customer);
                   break;
               }
        case 0:{
                   printf("Request Code 0: customer %d doesn't request\n", customer);
                   break;
               }
        case -1:{
                    printf("Request Code -1: customer %d's request fails since request exceeds need\n", customer);
                    break;
                }
        case -2:{
                    printf("Request Code -2: customer %d's request fails since request exceeds available\n", customer);
                    break;
                }
        case -3:{
                    printf("Request Code -3: customer %d's request fails since the state is unsafe\n", customer);
                    break;
                }
        default: exit(-1); break;
        }
    }
    else{
        switch(value){
        case 1:{
                   printf("Release Code 1: customer %d's release succeeds\n", customer);
                   break;
               }
        case 0:{
                   printf("Release Code 0: customer %d doesn't release any resource\n", customer);
                   break;
               }
        case -1:{
                    printf("Release Code -1: customer %d's release fails since release exceeds allocation\n", customer);
                    break;
                }
        default: exit(-2); break;
        }
    }
    
    //print system
    printf("current state\n");
    printf("available\n");
    printf("resource    %d  %d  %d\n", available[0], available[1], available[2]);
    printf("            maximum    allocation   need\n");
    for(i = 0; i < NUM_OF_CUSTOMERS; i++){
        printf("customer %d  %d  %d  %d    ", i, maximum[i][0], maximum[i][1], maximum[i][2]);
        printf("%d  %d  %d      ", allocation[i][0], allocation[i][1], allocation[i][2]);
        printf("%d  %d  %d\n", need[i][0], need[i][1], need[i][2]);
    }
    printf("\n");
}
