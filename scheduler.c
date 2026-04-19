//Rylee Roseenlieb
//Hessa Al Ali
//Operating Systems Project
//Phase 4

//include all the necessary header files
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <signal.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <semaphore.h>

//include header files of the other c files whoms functions are called in this c file
#include "scheduler.h"
#include "myshell.h"
#include "helper_Functions.h"
#include "shell_Commands.h"

//declare the global variables
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER; //the mutex for enqueuing and dequeing tasks into/from the queue
int incoming = 0; //global variable to signify the scheduler for a pre-emption
struct Task* currentTask = NULL; //global task variable to be used in the scheduler function and enqueue function
Queue taskQueue; //the global queue to be used in the enqueue and dequeue functions

//function to initialize queue
void initializeQueue(Queue *q) {
    //set the front and rear values to be -1 to signify an empty queue
    q->front = -1;
    q->rear = -1;    
}

//function that calls initialize queue
void callInitializeQueue(){
    initializeQueue(&taskQueue); //call the function that initalizes the queue
} 

//function to check if the queue is empty
int isQueueEmpty(Queue* q) {    
    if (q->front == -1){ //if the front value is negative one, then that means the queue is empty
        return true; //so return true
    }
    else{
        return false; //otherwise return false
    }
}

//function to check if the queue is full
int isQueueFull(Queue* q) {
    return q->rear == MAX_BUFFER - 1; //if the rear value is equal to the size of the queue - 1 (last index value), then return true, otherwise false
}

//the function that adds a task to the queue
void enqueue(Queue* q, struct Task* task) {
    pthread_mutex_lock(&queue_mutex); //lock the queue mutex lock so that a task cannot be dequeued from the queue

    if (isQueueFull(q)) { //first check if the queue is full
        //if it is
        pthread_mutex_unlock(&queue_mutex); //unlock the mutex
        return; //and return (exit the function)
    }

    if (currentTask){ //if the currentTask pointer is not NULL
        if (currentTask->burst_time > task->burst_time){ //compare the burst time of the tasks currently in the scheduler to the one being added
            //if the new task burst time is less than the current one's, then there's a pre-emption
            incoming = 1; //signify pre-emption by changing the "incoming" global variable value to 1
        }        
    }    

    //if this is the very first task being added to the queue
    if (q->front == -1) {
        q->front = 0; //set the front to 0
        q->rear = 0; //set the back to 0
        q->tasks[q->rear] = task; //insert the task at index 0
        pthread_mutex_unlock(&queue_mutex); //unlock the queue mutex
        return; //exit the function
    }

    //if this is not the very first task being added to the queue
    //insert task in sorted order
    int i; //create a counter variable

    if (currentTask->id == task->id){ //first check if the taskt being enqueued was just the one that was ran in the scheduler
        //if yes, then only allow it to be inserted up until one index before the front index of the queue
        for (i = q->rear; i > q->front; i--) { //start from the back of the queue, and iterate towards the front of it
            if (q->tasks[i]->burst_time > task->burst_time) { //if the new task's burst time is less than the burst time of the task in the index
                q->tasks[i + 1] = q->tasks[i]; //shift the task one index to the right (1 position "higher" in the queue)
            } else {
                break; //Found the correct position
            }
        }   
    }
    else {
        //otherwise allow the task to be inserted even in the front of the queue
        for (i = q->rear; i >= q->front; i--) { //start from the back of the queue, and iterate towards the front of it
            if (q->tasks[i]->burst_time > task->burst_time) { //if the new task's burst time is less than the burst time of the task in the index
                q->tasks[i + 1] = q->tasks[i]; //shift the task one index to the right (1 position "higher" in the queue)
            } else {
                break; //Found the correct position
            }
        }
    }

    // Place the new task
    q->tasks[i + 1] = task; //place the task in the empty spot in the queue
    q->rear++; //Update rear index

    pthread_mutex_unlock(&queue_mutex); //unlock the queue mutex
}

//the function that extracts the next task to run in the scheduler
struct Task* dequeue(Queue* q) {
    pthread_mutex_lock(&queue_mutex); //lock the queue mutex lock so no task can be added to the queue
    if (isQueueEmpty(q)) { //first check that the queue is not empty, if it is...  
        pthread_mutex_unlock(&queue_mutex); //unlock the mutex
        return NULL; //and return NULL
    }

    struct Task *task = q->tasks[q->front]; //create a task pointer and have it pointer to the task at the beginning of the queue
    if (q->front == q->rear) {  // Last task in the queue
        q->front = -1; //reset the front to -1 and the rear to -1 to indicate an empty queue
        q->rear = -1;
    } else {
        q->front++; //otherwise just increase the front index value
    }
    pthread_mutex_unlock(&queue_mutex); //unlock the mutex
    return task; //and return the task pointer
}

//the thread scheduler function
void* scheduler(void* arg) {
    callInitializeQueue(); //call the function that to initialize the queue
    int time = 0; //create a variable to store the time of the burst
    int i; //create a variable that will be a counter to the time quantum
    currentTask = (struct Task*)malloc(sizeof(struct Task)); //dynamic allocated memory for a placeholder currentTask
    if (currentTask == NULL) { //check if the memory was allocated
        perror("Failed to allocate memory for currentTask");
        exit(EXIT_FAILURE);
    }

    // Initialize the fields of currentTask
    currentTask->id = 0;         //set to ID because it is not a real task, but a placeholder current task
    currentTask->burst_time = 1000; // Set burst_time to 1000


    while (1) { //enter an infinite loop to run the scheduler until the server shuts down
        if (!isQueueEmpty(&taskQueue)) { //if the queue is not empty
            currentTask = dequeue(&taskQueue); //dequeue a task from the queue and have currentTask pointing to that dequeued task

            if (currentTask ->condition == 0){ //condition 0 indicates this is the first time the task is running in the scheduler
                printf("(%d)--- \033[1;32mstarted\033[0m (%d)\n", currentTask ->id, currentTask ->burst_time); //print that the task has been started
                currentTask->condition = 1; //change the condition signifier to 1
            }

            if (currentTask->burst_time < currentTask->time_quantum){ //check if the burst time is less than the alloted time quantum for the task
                //if yes then set the time for the scheduler to run the task be the remaining burst-time
                time = currentTask ->burst_time;
            }
            else{
                //otherwise make the time be the time quantum
                time = currentTask ->time_quantum;
            }

            i = 0; //set the counter to 0

            // Unlock the task's semaphore to let it run
            sem_post(currentTask ->semaphore);

            printf("(%d)--- \033[1;32mrunning\033[0m (%d)\n", currentTask ->id, currentTask ->burst_time); //print that the task is running

            while (incoming == 0 && i < time){ //check that there is not a pre-emption event and the counter is less than the time quantum
                sleep(1); //sleep for one second
                i++; //increase the counter
                currentTask->burst_time -= 1; //decrease the burst_time by 1 in the currentTask struct
            }
 
            //Lock the semaphore again to stop the task from running further
            sem_wait(currentTask ->semaphore);

            incoming = 0; //reset the pre-emption variable


            //Re-enqueue the task if it still has burst time left (simulating time slicing)            
            if (currentTask ->burst_time > 0) { //if there is still time left
                currentTask->time_quantum = 7; //change the time quantum to 7
                enqueue(&taskQueue, currentTask); //re-enqueue the task             
                printf("(%d)--- \033[1;33mwaiting\033[0m (%d)\n", currentTask ->id, currentTask ->burst_time); //print that the task is now waiting
            } else { 
                //otherwise if the burst time is 0
                printf("(%d)--- \033[1;31mended\033[0m (%d)\n", currentTask ->id, currentTask ->burst_time); //print that the task ended 
                sem_destroy(currentTask->semaphore);//Destroy the semaphore 
                free(currentTask->semaphore); //free the semaphore
                free(currentTask); //free the task
            }
        } else {            
            sleep(1); //sleep for one second before trying to see if there is at least one task in the queue to run
        }
    }

    return NULL; //return NULL to safely exit the function
}

//the thread function to execute the running demo code
void* executableTask(void* arg) {
    struct clientThread* client_thread = (struct clientThread*)arg; //cast the passed thread argument and store in the client thread structure
    char *args[MAX_BUFFER]; //create a buffer to store the individual arguments of the command
    char outputBuffer[MAX_BUFFER]; //create a buffer to store the output that will be sent to the client
    size_t totalBytesSent = 0; //create a variable of type size_t in order to calculate and store the number of bytes being sent to the client

    parseInput(client_thread->args, args); //call the parseInput function to separate the arguments of the thread (separate ./demo from its burst_time integer)

    pid_t pid = fork(); //fork a child
    if (pid == 0) {
        // In child process: execute the demo program
        if (execvp(args[0], args) == -1) { //check if the execution failed and print an error and exit if it did
            perror("execvp failed");
            exit(EXIT_FAILURE); // Exit child process on failure
        }
    } else if (pid < 0) { //check if the fork failed and print an error if so
        perror("fork failed");
        pthread_exit(NULL); //exit the thread
    }

    int i = 1; //create a counter variable starting at 1
    while (i <= client_thread -> burst_time){  //while the counter is less than the burst time of the task 

        sem_wait(client_thread->semaphore); //call wait to be able to access critical section when the semaphore is equal to 1       
        snprintf(outputBuffer, MAX_BUFFER, "demo %d/%d\n", i, client_thread->burst_time); //save the "demo int/burst_time" to the outputBuffer
        send(client_thread->socket, outputBuffer, MAX_BUFFER, 0); //send the outputBuffer to the client
        totalBytesSent += strlen(outputBuffer); //get the number of bytes in the outputBuffer and add it to the total number of bytes
        i++; //increase the coungter by 1
        sem_post(client_thread->semaphore); //post the semaphore (unlock it)
        sleep(1); //and sleep for one second
        //want to unlock the semaphore to give the scheduler an opportunity to lock the semaphore/stop the task from running

    }

    //wait for the child process to return to the parent
    int status; 
    waitpid(pid, &status, 0);

    strncpy(outputBuffer,"done",MAX_BUFFER); //copy into the outputBuffer the string "done"
    send(client_thread->socket, outputBuffer, MAX_BUFFER, 0); //send the output buffer to the client

    printf("[%d]<<< %ld bytes sent\n", client_thread->clientID, totalBytesSent); //print out the total number of bytes that was sent to the client
       
    free(client_thread); //free the memory allocated for the client_thread struct
    pthread_exit(NULL); //exit the thread
}

//function that handles creating the task for the queue and creating the thread to execute ./demo
void add_to_queue(struct newExecutable *item) {    
    // Dynamically allocate memory for the new Task
    struct Task *newTask = (struct Task *)malloc(sizeof(struct Task));
    if (newTask == NULL) { //check that the memory was allocated properly
        perror("Failed to allocate memory for Task");
        return;
    }

    // Dynamically allocate memory for the semaphore
    sem_t *shared_semaphore = (sem_t *)malloc(sizeof(sem_t));
    if (shared_semaphore == NULL) { //check that the memory was allocated properly
        perror("Failed to allocate memory for semaphore");
        free(newTask);
        return;
    }
    sem_init(shared_semaphore, 0, 0); //initiliaze the semaphore, with the initial value of 0 (locked)

    // Initialize the Task    
    newTask->id = item->clientID; //add the client id from the function's passed argument
    newTask->burst_time = getBurstTime(item->program); //assign the burst-time
    newTask->semaphore = shared_semaphore;  //assign the shared semaphore
    newTask->condition = 0; //set the condition that checks if it has already ran once in the scheduler to 0
    newTask->time_quantum = 3; //set the initial time quantum to 3


    // Enqueue the new task
    enqueue(&taskQueue, newTask);    

    // Dynamically allocate memory for the clientThread (arguments for the client thread function)
    struct clientThread *client_thread = (struct clientThread *)malloc(sizeof(struct clientThread));
    if (client_thread == NULL) { //check that the memory was allocated properly
        perror("Failed to allocate memory for clientThread");
        free(newTask); //free the previously created items 
        free(shared_semaphore);
        return;
    }

    // Initialize clientThread using newTask data
    client_thread->socket = item->socket; //assign the client socket to be the same as the item that was passed to the function
    client_thread->address = item->address; //assign the client address to be the same as the item that was passed to the function
    strncpy(client_thread->args, item->program, MAX_BUFFER); //copy the buffer from the passed item into the client_thread args buffer
    client_thread->burst_time = newTask -> burst_time; //assign the same burst time as the newTask
    client_thread->semaphore = shared_semaphore;  // Share the same semaphore as the newTask
    client_thread->clientID = newTask->id; //assign the same client id as in the newTask

    // Create a thread for the executable task
    pthread_t execThread;
    if (pthread_create(&execThread, NULL, executableTask, (void *)client_thread) != 0) { //check that the thread was successfully created; pass client_thread as the argument
        perror("Failed to create thread");
        //free all the dynamically created memory items if there is a failure
        free(client_thread);
        free(newTask);
        free(shared_semaphore);
        return;
    }

    pthread_detach(execThread);  //Detach the thread so it is handled independently

    printf("(%d)--- \033[38;5;33mcreated\033[0m (%d)\n", client_thread->clientID, client_thread->burst_time); //print that the task was created

    free(item); //free the memory of "item", which is the passed argument to this function
}

//function to extract the burst time from the ./demo executable
int getBurstTime(char* buffer) {
    char *token;  // Variable to temporarily store command, argument, or symbol when parsing
    char *saveptr;  // Pointer to keep track of position in the string

    int burstTime = 0; //create variable to store the found burst time, intialize it to 0 if no integer value is in the buffer

    // Remove the trailing newline character, if any
    buffer[strcspn(buffer, "\n")] = '\0';

    // Tokenize the input buffer
    token = strtok_r(buffer, " ", &saveptr); //token ./demo part first 
    if (token != NULL) {
        token = strtok_r(NULL, " ", &saveptr);  // Get the second token (burst time)
        if (token != NULL) {
            burstTime = atoi(token);  // Convert the second token to an integer and store in burst time variable
        }
    }    
    return burstTime;
}


