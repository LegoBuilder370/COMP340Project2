/**
 * Header file for dining philosophers
 */

 #include <pthread.h>
 #include <semaphore.h>  
 #include <stdlib.h>
 #include <unistd.h>
 #include <stdio.h>
 
 // the number of philosophers
 #define NUMBER 5
 
 // the maximum number of random numbers
 #define MAX_LENGTH 500
 
 // the array holding the list of random numbers 
 int rand_numbers[MAX_LENGTH];
 
 //function for getting a random number from the list
 int get_next_number();
 
 //mutex lock to use in order to protect the order of random numbers
 pthread_mutex_t mutex_rand;
 
 //position of next random number 
 int rand_position;
 
 // the state of each philosopher (THINKING, HUNGRY, EATING)
 enum {THINKING, HUNGRY, EATING} state[NUMBER];
 
 // the id of each philosopher (0 .. NUMBER - 1)
 int thread_id[NUMBER];
 
 // semaphore variables and associated mutex lock
 sem_t		sem_vars[NUMBER];
 pthread_mutex_t 	mutex_lock;
 
 //function that simulates the philosopher operation
 void *philosopher(void *param);
 
 //function for the philosopher to pickup the chopsticks
 void pickup_chopsticks(int number);
 
 //function for the philosopher to return the chopsticks
 void return_chopsticks(int number);

 //added to header
 void test(int i);
//end of header file


int main(int argc, char const *argv[])
{
    rand_position = 0;

    // Read file into array
    FILE *myFile;
    myFile = fopen(argv[1], "r");


    if (myFile == NULL){
        printf("Error Reading File\n");
        exit (0);
    }

    for (int i = 0; i < MAX_LENGTH; i++){
        fscanf(myFile, "%d,", &rand_numbers[i] );
    }

    for (int i = 0; i < MAX_LENGTH; i++){
        printf("Number is: %d\n\n", rand_numbers[i]);
    }

    fclose(myFile);

    printf("Array made\n");
    
    pthread_mutex_init(&mutex_lock, NULL);

    //set up semaphores
    for (size_t i = 0; i < NUMBER; i++)
    {
        sem_t mutex;
        sem_init(&mutex, 0, 1);
        sem_vars[NUMBER] = mutex;
    }

    printf("set up semaphores\n");

    pthread_t tid[NUMBER];
    for (size_t i = 0; i < NUMBER; i++)
    {
        thread_id[i] = i;
        pthread_create(&tid[i], NULL, philosopher, &(thread_id[i]));
    }

    printf("Created threads\n");

    for (size_t i = 0; i < NUMBER; i++)
    {
        printf("Joining threads\n");
        pthread_join(tid[i], NULL);
    }

    printf("Joined threads\n");

    // delete semaphores
    for (size_t i = 0; i < NUMBER; i++)
    {
        sem_destroy(&sem_vars[NUMBER]);
    }

    printf("Destroyed semaphores\n");

    pthread_mutex_destroy(&mutex_lock);

    printf("Destroyed mutex\n");

    return 0;
}

void *philosopher(void *param) {
    printf("Entered philosopher\n");
    int threadNum = (*(int*)param);
    int bitesTaken = 0;
    state[threadNum] = THINKING;

    while (bitesTaken < 5) {
        sleep(get_next_number());
        // sleep(10);
        pickup_chopsticks(threadNum);
        sleep(get_next_number());
        // sleep(10);
        return_chopsticks(threadNum);
        bitesTaken++; 
    }

    pthread_exit(0);
}


void pickup_chopsticks(int i) {
    pthread_mutex_lock(&mutex_lock);
    state[i] = HUNGRY;
    test(i); // test self
    pthread_mutex_unlock(&mutex_lock);
    sem_wait(&sem_vars[NUMBER]);
}

void return_chopsticks(int i) {
    pthread_mutex_lock(&mutex_lock);
    state[i] = THINKING;
    test((i+NUMBER-1)%NUMBER); // test LEFT
    test((i+1)%NUMBER); // test RIGHT
    pthread_mutex_unlock(&mutex_lock);
}

void test(int i){
    if(state[i] == HUNGRY && state[(i+NUMBER-1)%NUMBER] != EATING && state[(i+1)%NUMBER] != EATING) {
        state[i] = EATING; // phi[i] can eat
        sem_post(&sem_vars[i]); // wake up phi[i] if it is blocked
    }    
}

int get_next_number() {
    return rand_numbers[rand_position++];
}