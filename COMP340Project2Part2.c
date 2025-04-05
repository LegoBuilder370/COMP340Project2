/*
  Author:  John Bauer, Christopher Kobulnicky
  Course:  COMP 340, Operating Systems
  Date:    4 April 2025
  Description:   This file implements the functionality for the philosopher's dining problem with an additional chopstick
  Compile with:  gcc -o proj2part2 COMP340Project2Part2.c
  Run with:      ./proj2part2 input2.out (or any input file you so desire)

*/



/**
 * Header file for dining philosophers
 */

 #include <pthread.h>
 #include <semaphore.h>  
 #include <stdlib.h>
 #include <unistd.h>
 #include <math.h>
 #include <stdio.h>
 #include <sys/time.h>
 
 // the number of philosophers
 #define NUMBER 10
 
 // the maximum number of random numbers
 #define MAX_LENGTH 500
 
 // the array holding the list of random numbers 
 int rand_numbers[MAX_LENGTH];
 
 //function for getting a random number from the list
 int get_next_number();

 // function that returns a random int between 1 and 5 inclusive
 int get_next_number_random();
 
 //mutex lock to use in order to protect the order of random numbers
 pthread_mutex_t mutex_rand;
 
 //position of next random number 
 int rand_position = -1;
 
 // the state of each philosopher (THINKING, HUNGRY, EATING)
 enum {THINKING, HUNGRY, EATING} state[NUMBER];

 // the state of the center chopstick
 int centerChopstick;
 int chopstick[NUMBER];
 
 // the id of each philosopher (0 .. NUMBER - 1)
 int thread_id[NUMBER];
 
 // semaphore variables and associated mutex lock
 sem_t sem_vars[NUMBER];
 pthread_mutex_t mutexlock;

 // chopstick locks
 pthread_mutex_t chopLocks[NUMBER];
 
 //function that simulates the philosopher operation
 void *philosopher(void *param);
 
 //function for the philosopher to pickup the chopsticks
 void pickup_chopsticks(int number);
 
 //function for the philosopher to return the chopsticks
 void return_chopsticks(int number);

 //added to header
 void test(int i);
//end of header file

double maxWaitTime;
double totalWaitTime;

int main(int argc, char const *argv[])
{
    FILE *myFile;
     myFile = fopen(argv[1], "r");
   
     if (myFile == NULL){
         printf("Error Reading File\n");
         exit (0);
     }
  
    for (int i = 0; i < MAX_LENGTH; i++){
        fscanf(myFile, "%d,", &rand_numbers[i] );
    }
  
    //  for (int i = 0; i < MAX_LENGTH; i++){
    //      printf("Number is: %d\n\n", rand_numbers[i]);
    //  }
  
    fclose(myFile);

    pthread_mutex_init(&mutexlock, NULL);
    
    for (size_t i = 0; i < NUMBER; i++)
    {
        state[i] = THINKING;
    }

    // set up chopsticks
    for (size_t i = 0; i < NUMBER; i++)
    {
        chopstick[i] = -1;
    }
    centerChopstick = -1;

    //set up semaphores
    sem_init(sem_vars, 0, 1);

    pthread_t tid[NUMBER];
    for (size_t i = 0; i < NUMBER; i++)
    {
        thread_id[i] = i;
        pthread_create(&tid[i], NULL, philosopher, &(thread_id[i]));
    }

    for (size_t i = 0; i < NUMBER; i++)
    {
        pthread_join(tid[i], NULL);
    }

    // delete semaphores
    for (size_t i = 0; i < NUMBER; i++)
    {
        sem_destroy(&sem_vars[NUMBER]);
    }

    pthread_mutex_destroy(&mutexlock);

    printf("Max wait time: %f\n", maxWaitTime);
    printf("Average wait time: %f\n", totalWaitTime / ((double) NUMBER*5.0));

    return 0;
}

void *philosopher(void *param) {
    int threadNum = (*(int*)param);
    int bitesTaken = 0;

    // printf("Philosipher %d is starting\n", threadNum);

    while (bitesTaken < 5) {
        sleep(get_next_number());
        // printf("Philosipher %d is done sleeping\n", threadNum);
        pickup_chopsticks(threadNum);
        
        sleep(get_next_number());
        return_chopsticks(threadNum);
        bitesTaken++;
        // printf("Philosipher %d has taken bite %d\n", threadNum, bitesTaken);
    }

    pthread_exit(0);
}


void pickup_chopsticks(int i) {
    int hasChopsticks = 0;

    state[i] = HUNGRY;

    struct timeval  before, after;
    double waitTime;
    gettimeofday(&before, NULL);

    while (hasChopsticks == 0) {
    pthread_mutex_lock(&mutexlock);
    //printf("%d has the lock\n", i);

    if (chopstick[i] == -1 && chopstick[(i+1)%NUMBER] == -1) {
        state[i] = EATING; // phi[i] can eat

        gettimeofday(&after, NULL);
        waitTime = (double)(after.tv_usec-before.tv_usec)/1000 + (double)(after.tv_sec-before.tv_sec)*1000;
        totalWaitTime += waitTime;
        if (waitTime > maxWaitTime) {
            maxWaitTime = waitTime;
        }

        chopstick[i] = i;
        chopstick[(i+1)%NUMBER] = i;

        // printf("Philosipher %d is currently eating with chopsticks %d and %d\n", i, i, (i+1) % NUMBER);

        hasChopsticks = 1;
        pthread_mutex_unlock(&mutexlock);
    }
    else if (chopstick[i] == -1 && centerChopstick == -1) {
        state[i] = EATING;

        gettimeofday(&after, NULL);
        waitTime = (double)(after.tv_usec-before.tv_usec)/1000 + (double)(after.tv_sec-before.tv_sec)*1000;
        totalWaitTime += waitTime;
        if (waitTime > maxWaitTime) {
            maxWaitTime = waitTime;
        }

        chopstick[i] = i;
        centerChopstick = i;

        // printf("Philosipher %d is currently eating with chopsticks %d and C\n", i, i);

        hasChopsticks = 1;
        pthread_mutex_unlock(&mutexlock);
    }
    else if (state[(i+1)%NUMBER] == -1 && centerChopstick == -1) {
        state[i] = EATING;

        gettimeofday(&after, NULL);
        waitTime = (double)(after.tv_usec-before.tv_usec)/1000 + (double)(after.tv_sec-before.tv_sec)*1000;
        totalWaitTime += waitTime;
        if (waitTime > maxWaitTime) {
            maxWaitTime = waitTime;
        }
        
        chopstick[(i+1)%NUMBER] = i;
        centerChopstick = i;

        // printf("Philosipher %d is currently eating with chopsticks %d and C\n", i, (i+1) % NUMBER);

        hasChopsticks = 1;
        pthread_mutex_unlock(&mutexlock);
    }
    else {
        // printf("Philosipher %d is waiting for chopsticks\n", i);
        pthread_mutex_unlock(&mutexlock);
        sem_wait(&sem_vars[i]);
    }
    }
}

void return_chopsticks(int i) {
    pthread_mutex_lock(&mutexlock);
    //printf("%d has the lock\n", i);
    state[i] = THINKING;

    if (chopstick[i] == i) {
        chopstick[i] = -1;

        // printf("Philosipher %d has returned chopstick %d\n", i, i);

        sem_post(&sem_vars[(i+NUMBER-1) % NUMBER]);
    }
    if (chopstick[(i+1)%NUMBER] == i) {
        chopstick[(i+1)%NUMBER] = -1;

        // printf("Philosipher %d has returned chopstick %d\n", i, (i+1)%NUMBER);

        sem_post(&sem_vars[(i+1) % NUMBER]);
    }
    if (centerChopstick == i) {
        centerChopstick = -1;

        // printf("Philosipher %d has returned chopstick C\n", i);

        for (size_t i = 0; i < NUMBER; i++)
        {
            sem_post(&sem_vars[i]);
        }
        
    }

    pthread_mutex_unlock(&mutexlock);
    //printf("%d released the lock\n", i);
}

void test(int i){
    if(state[i] == HUNGRY && state[(i+NUMBER-1)%NUMBER] != EATING && state[(i+1)%NUMBER] != EATING) {
        state[i] = EATING; // phi[i] can eat
    } 
}

//TODO: change
// int get_next_number() {
//     int toReturn = (int) (random() * 5 / ((double)RAND_MAX + 1)) + 1;
//     return toReturn;
// }

int get_next_number() {
    pthread_mutex_lock(&mutex_rand);
    rand_position = (rand_position + 1) % MAX_LENGTH;
    pthread_mutex_unlock(&mutex_rand);
    // printf("rand_position after: %d\n", rand_position);

    return rand_numbers[rand_position];
}
