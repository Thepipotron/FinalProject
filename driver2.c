

#include <stdio.h>
#include<stdlib.h>
#include<stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <semaphore.h>

//this variable contains the coaches 1 means a coach is avalible (on his phone) 0 means he is busy with customer
#define WAITING_ROOM "/wroom"
#define COACHES "/trainers"
#define FIRST "/pointer"
#define ALLOC "/allocation"
#define SEM "sema"
#define MAX_SIZE sizeof(int) * 8

int w;
int c;
int p;
int a;

int a_size;
int c_size;
int w_size;
// int sets;


/**
 * 
 */
int allocT_access(int col, int row, int* arr){
    int final = row + col * 6;
    return arr[final];
}

void allocT_change(int col, int row,int to, int* arr){
    int final = row + col * 6;
    arr[final] = to;
}



/** The purpose of this function is to check the avalibilty of the trainers or the waiting room
 *  @param array the array to check
 *  @param size the size of the array
 *  @param start the first seat currently
 *  @return the index of the avalible spot or a -1 if no spot is avalible
 */
int avalible(int* array, int size, int *start){

    for(int i = *start; i < size; i++){
        if(array[i] == 0){
            array[i] = 1;
            return i;
        }
    }

    for(int i = 0; i < *start; i++){
        if(array[i] == 0){
            array[i] = 1;
            return i;
        }
    }

    return -1;
}

void print_info(int *coaches, int *waiting_room){

    printf("Coaches:\n");
    for(int i = 0; i < c_size; i++){

        printf("%d ",coaches[i]);

    }
    printf("\nWaiting Room:\n");
    for(int i = 0; i < w_size; i++){

        printf("%d ",waiting_room[i]);

    }
    printf("\n");
}

/** The purpose of this function is to find the first customer waiting in the waiting room
 *  @param array the array to search for an open spot
 *  @param first the seat in the waiting room
 *  @param size the size of array
 *  @return the index of the first avalible customer, -1 if the waiting room is empty
 */
int first_cus(int *array, int *first, int size){

    for(int i = *first; i < w_size; i++){
        if(array[i] == 1){
            //incrementing who is first in line
            (*first)++;
            if(*first >= w_size){

                *first = 0;
            }
            return i;
        }
    }

    for(int i = 0; i < *first; i++){
        if(array[i] == 1){
            //incrementing who is first in line
            (*first)++;
            if(*first >= w_size){
                *first = 0;
            }
            return i;
        }
    }

    return -1;
}

// int sets(int *set){
//       set = (rand() % 8);
//       if (set = 1){
//             return -1;
//       }
//       else{
//             return 0;
//       }
// }
int customers(int *coaches, int *waiting_room, int *first,sem_t *sema, int *allocationTable){

    int tmp = 0;

    sem_wait(sema);
    int trainer = avalible(coaches,c_size,&tmp);


    //checking to see if trainers are avalible
    if(trainer == -1){



        //if no trainer avilible walk back to the waiting room
        sleep(0.1);
        int cust_wait = avalible(waiting_room,w_size, first);


        //if the waiting room is full
        if(cust_wait == -1){
            //leave the gym
            exit(0);
        }

        printf("Moving customer to waiting room...\n");
        print_info(coaches,waiting_room);
        fflush(stdout);
        sem_post(sema);

        //waiting for a trainer to come get them
        while(waiting_room[cust_wait] == 1);

        sem_wait(sema);

        //getting the trainer and releasing it to the customer as well as the seat in the waiting room
        trainer = waiting_room[cust_wait] * -1;
        waiting_room[cust_wait] = 0;
    }


    //ensure the trainer is marked as busy
    coaches[trainer] = 1;
    printf("Attaching trainer %d to cust\n", trainer);
    print_info(coaches,waiting_room);
    fflush(stdout);
    sem_post(sema);

    //once the trainer gets the customer
    //The trainer needs to select the amount of sets the Customer will do
    //the amount of sets will be from a random number from 2 to 7 sets.
    
    int set[] = {2, 3, 4, 5, 6, 7};
    int random = (rand() % 6);
    int numberSets = set[random];
    printf("Number of Sets for the Customer: %d \n" , numberSets);

    // int plates[]= {2.5, 5, 10, 25, 35, 45};
    // int random = (rand() % 6);
    // int customerPlates = plates[random];
    // printf("The Customer is using this plate: %d \n", customerPlates);
    

    //train the customer ** the training time for each customer is decided by how many sets
    if (random = 2){
        sleep(1);
    }
    else if(random = 3){
        sleep(1.5);
    }
    else if(random = 4){
        sleep(2);
    }
    else if(random = 5){
        sleep(2.5);
    }
    else if(random = 6){
        sleep(3);
    }
    else if(random = 7){
        sleep(3.5);
    }
    else
    {
        exit(0);
    }
    //next trainer walks back to the waiting room
    sleep(.05);

    sem_wait(sema);
    //trainer searches for a new customer in the waiting room
    int new_cust = first_cus(waiting_room, first, w_size);

    printf("cust done training\n");
    fflush(stdout);

    //no customers waiting
    if(new_cust == -1){
        //walking back to the gym
        sleep(.05);


        //mark trainer as ready
        coaches[trainer] = 0;
        printf("Releasing Trainer #%d\n",trainer);
        print_info(coaches,waiting_room);
        fflush(stdout);
        sem_post(sema);

    }

    else{

        //linking the trainer with a waiting customer in the waiting room
        printf("moving Trainer #%d to custromer #%d\n",trainer, new_cust);
        printf("First: %d\n", *first);
        fflush(stdout);
        waiting_room[new_cust] = trainer * -1;
        sem_post(sema);
    }
}

/** The purpose of this function is to detect when deadlock occurs and return to a safe state using the banker's algorithm
 *  @param process the process in this algorithm acts as the Customer
 *  @param temp this is a temporary variable
 *  @param resource the resources acts as the weights
 *  @return the index of the first avalible customer, -1 if the waiting room is empty
 */

int deadlockDetection(int process, int temp, int resource, int *allocationTable){
    // This is old code, but will leave in incase I need to use it
    // srand(time(0));
    // char* typeWeight[] = {"2.5", "5", "7.5", "10", "12.5", "15"};  This is old code, but will leave in incase I need to use it
    // int amountWeight[6];
    for (int i = 0; i<6; i++){
        // setting the the amount of weights in the facility to 10 of each weight type
        //This will change each value in the column to a value of of 1-10 for the weights after the amount and different types
        // int rowChange = i + 2; 
        // rowChange <= 6;
        allocT_change(i,1,10,allocationTable);
        for(int j = 2; j<9; j++){
            allocT_change(i,j,(rand() % 10),allocationTable); // This adds how many of each weight the Customer is trying to use
        }
        allocT_change(i,0,(2.5+(i * 2.5)),allocationTable); // This adds the types of weights to the first row column
          // This sets the amount of each weight available in the second row column
        // printf( "The Customer is using %d of %s lb weights. \n", amountWeight[i], typeWeight[i]);  old code
    }   
    int count = 0;
    int i;
    int j;

    //The available is the plates not being used
    //The Current is the plates being used
    //The Maximum claim is the maximum plates that can be used **NEED TO CHANGE THIS TO READ FROM THE 2ND COLLUMN
    int available[5], current[2][5], maximum_claim[1][5]; // Need to 
    int maximum_resources[5], running[5], safe_state = 0;

    for(int col = 0; col < 6; col++){
        for(int row = 2; row < 8; row++){       
            
            process = allocT_access(col,row,allocationTable);
        }
    }
    // process = allocT_access(); //Don't know how to implement this part

    for(i = 0; i < process; i++) {
        running[i] = 1;
        count++;
    }

    // This is the weights currently being used by the Customers
    for(i = 0; i < 6; i++){
        for(j = 2; j < 5; j++){
            current[2][5] = allocT_access(i,j,allocationTable);
        }
    }

    for(i = 0; i < 6; i++){
        available[5] = allocT_access(i,1,allocationTable);
    }
          
    // resource = *allocationTable;

    for(i = 0; i < resource; i++) { 
        allocT_change(i,1,i,allocationTable);
    }
    // change the current weights being used
    for(i = 0; i < process; i++) {
        for(j = 2; j < resource; j++) 
        {
            allocT_change(i,j,current[j][i],allocationTable);
        }
    }
    // Creating the maximum claim 
    for(i = 0; i < process; i++) {
        for(j = 0; j < resource; j++) 
        {
                maximum_claim[i][j] = allocT_access(j,1,allocationTable);
        }
    }

    for(i = 0; i < process; i++) {
        for(j = 0; j < resource; j++) {
                allocationTable[j] = allocationTable[j] + current[i][j];
        }
    }

    for(i = 0; i< resource; i++) {
        available[i] = maximum_resources[i] - allocationTable[i];
    }

    while(count != 0) {
        safe_state = 0;
        
        for(i = 0; i < process; i++) {
                if(running[i]) {
                    temp = 1;
                    for(int j = 0; j < resource; j++) {
                            if(maximum_claim[i][j] - current[i][j] > available[j]) {
                                temp = 0;
                                break;
                            }
                    }
                    if(temp) {
                            running[i] = 0;
                            count--;
                            safe_state = 1;
                            for(int j = 0; j < resource; j++) {
                                    available[j] = available[j] + current[i][j];
                            }
                            break;
                    }
                }
        }
        if(!safe_state) {
                printf("\nThe processes are experiencing deadlock \n");
                break;
        } 
        else {
                printf("\nThe processes are running correctly \n");
                for(i = 0; i < resource; i++) 
                {
                    printf("\t%d", allocT_access(i,1,allocationTable)); // Prints off how many of the weights are available to show deadlock is not occuring
                }
                printf("\n");
        }
    }
    return 0;
}

/** The purpose of this function is to create a shared memory space for the custromers
 *  to all have acces to, one for coaches and one for trainers (called AFTER fork())
 *  @param wroom_size the size of the waiting room
 *  @param trainer_size the number of trainers
 *  @return 0 on success -1 on fail
 */
int ini_ipc(int wroom_size, int trainer_size){

    //first creat or open a shared memory space for the coaches
    c = shm_open(COACHES, O_CREAT | O_RDWR, 0666);
    if(c < 1){
        perror("err");
    }
    ftruncate(c, sizeof(int) * trainer_size);

    int *coaches = malloc(sizeof(int) * MAX_SIZE);
    coaches = (int*)mmap(0,sizeof(int) * MAX_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, c, 0);

    for(int i = 0; i < c_size; i++){
        coaches[i] = 0;
    }

    //now create the shared space for the customer waiting room
    w = shm_open(WAITING_ROOM, O_CREAT | O_RDWR, 0666);
    if(w < 1){
        perror("err");
    }
    ftruncate(w, sizeof(int) * wroom_size);

    //finally put the size of each of the spaces into global variables so they are shared among procs
    p = shm_open(FIRST,O_CREAT | O_RDWR, 0666);


    a = shm_open(ALLOC, O_CREAT | O_RDWR, 0666);
    ftruncate(a,sizeof(int) * 8 * 6);

    int *allocationTable = (int*) mmap(0,sizeof(int) * 8 * 6, PROT_READ | PROT_WRITE, MAP_SHARED, a, 0);
    for(int i = 0; i < 6*8; i++){
        allocationTable[i] = 0;
    }


    ftruncate(p,sizeof(int));
    int *first = (int*) mmap(0,sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, p, 0);
    *first = 0;

    return 0;
}



int main(){

    shm_unlink(FIRST);
    shm_unlink(COACHES);
    shm_unlink(WAITING_ROOM);
    shm_unlink(ALLOC);

    sem_unlink (SEM);   

    //if a coach is avalible 0
    int *coaches;
    c_size = 3;

    //this variable contains customers in the waiting room. 0 means the seat is open 1 means the seat is occupied
    int *waiting_room;
    w_size = 6;

    int *allocationTable;
    a_size = c_size;

    coaches = malloc(sizeof(int) * MAX_SIZE);
    waiting_room = malloc(sizeof(int) * MAX_SIZE);

    sem_t *sema;
    if((sema = sem_open(SEM, O_CREAT | O_EXCL, 0644, 1)) < 0 ){
        perror("error");
    }


    int* first = malloc(sizeof(int));

    //first initialization of the shared memory space
    ini_ipc(8,8);

    int b = fork();
    int weights = allocT_access(0,0,allocationTable);
    if(b == 0){

        int act = shm_open(ALLOC, O_RDWR, 0666);
        allocationTable = (int*) mmap(0,sizeof(int) * 6 * 8, PROT_READ | PROT_WRITE, MAP_SHARED, act, 0);


        deadlockDetection(c_size, 1, weights, allocationTable); //when adding this value, the program does not stop running
        exit(0);
    }

    //loop that creates custromers, for testing purposes we are just going to create 3 for now
    for(int i = 0; i < 18; i++){

        //split of a child proc
        int pid = fork();

        //execute customer function then leave
        if(pid == 0){

            //initialize the coaches memory space
            int coach = shm_open(COACHES, O_RDWR, 0666);
            coaches = (int*) mmap(0,sizeof(int) * c_size, PROT_READ | PROT_WRITE, MAP_SHARED, coach,0);

            //initialize the waiting room memory space
            int waiting = shm_open(WAITING_ROOM, O_RDWR, 0666);
            waiting_room = (int*) mmap(0,sizeof(int) * w_size, PROT_READ | PROT_WRITE, MAP_SHARED, waiting, 0);

            //initialize a variable to keep track of whose first in line
            int fir = shm_open(FIRST, O_RDWR, 0666);
            first = (int*) mmap(0,sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fir, 0);

            int act = shm_open(ALLOC, O_RDWR, 0666);
            allocationTable = (int*) mmap(0,sizeof(int) * 8 * 6, PROT_READ | PROT_WRITE, MAP_SHARED, act, 0);

            customers(coaches,waiting_room, first, sema, allocationTable);

            exit(0);

        }
        double rand_sleep = (double)rand()/RAND_MAX *1.5;
        sleep(rand_sleep);
    }

    //initialize the coaches memory space
    int coach = shm_open(COACHES, O_RDWR, 0666);
    if(coach < 1){
        perror("err");
    }
    coaches = (int*) mmap(0,sizeof(int) * c_size, PROT_READ | PROT_WRITE, MAP_SHARED, coach,0);

    int waiting = shm_open(WAITING_ROOM, O_RDWR, 0666);
    waiting_room = (int*) mmap(0,sizeof(int) * w_size, PROT_READ | PROT_WRITE, MAP_SHARED, waiting, 0);

    int fir = shm_open(FIRST, O_RDWR, 0666);
    first = (int*) mmap(0,sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fir, 0);

    int act = shm_open(ALLOC, O_RDWR, 0666);
    allocationTable = (int*) mmap(0,sizeof(int) * 6 * 8, PROT_READ | PROT_WRITE, MAP_SHARED, act, 0);

    int status = 0;

    //wait for all child proc to end
    while(wait(&status) > 0);

    printf("Coaches:\n");
    for(int i = 0; i < c_size; i++){

        printf("%d ",coaches[i]);

    }
    printf("\nWaiting Room:\n");
    for(int i = 0; i < w_size; i++){

        printf("%d ",waiting_room[i]);

    }

    printf("\n\n");

    for(int i = 0; i < 8; i++){
        for(int j = 0; j < 6; j++){
            printf(" %d ",allocT_access(j,i,allocationTable));
        }
        printf("\n");
    }

    printf("\n");

    printf("First: %d\n",*first);

    close(c);
    close(w);
    close(p);
    close(a);
    shm_unlink(FIRST);
    shm_unlink(COACHES);
    shm_unlink(WAITING_ROOM);
    shm_unlink(ALLOC);

    sem_unlink (SEM);   
    sem_close(sema); 

}