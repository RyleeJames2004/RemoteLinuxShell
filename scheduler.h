//Rylee Roseenlieb
//Hessa Al Ali
//Operating Systems Project
//Phase 4


#define MAX_BUFFER 1000 

// Struct to hold client data (socket and address)
struct ClientData{
    int socket; //to hold the value of the client socket
    struct sockaddr_in address; //to hold the struct that has the client socket address information
    int clientID; //variable to hold the clients ID
};

//the structure to include all the necessary information about a task for the scheduler/queue
struct Task {    
    int id; //the id of the client whom the task is for
    int burst_time; //the (remaining) time for the burst
    sem_t* semaphore; //the shared semaphore with the client thread structure
    int condition; //this variable determines if this is the tasks first time in the scheduler or subsequent time
    int time_quantum; //the amount of time the scheduler should "run" that task
};

//structure to include necessary information for the Task and clientThread structures to be created and intialized
struct newExecutable {
    int socket; //the client's socket
    struct sockaddr_in address; //the client's [server] address
    char program[MAX_BUFFER]; //the .demo input
    int clientID; //the client's assigned id
};

//the structure to include the necessary information for the client thread (executes the ./demo functionality and printing)
struct clientThread {
    int socket; //the client's socket
    struct sockaddr_in address; //the address of the client
    int clientID; //the client's assigned id
    char args[MAX_BUFFER]; //the ./demo [integer value] input
    int burst_time; //the burst-time of the demo executable
    sem_t* semaphore; //the shared semaphore with the corresponding task structure in the scheduler
};

//the structure to hold the all the necessary items of the queue
typedef struct {
    struct Task *tasks[MAX_BUFFER];  // Array to hold the elements of the queue
    int front;     // Front index of the queue
    int rear;      // Rear index of the queue
} Queue; 

//declare the functions  
void add_to_queue(struct newExecutable *item); //function that handles creating the task for the queue and creating the thread to execute ./demo
void* scheduler(void* arg); //the thread scheduler function
void* executableTask(void* arg); //the thread function to execute the running demo code
int getBurstTime(char* buffer); //function to extract the burst time from the ./demo executable
int isQueueEmpty(Queue* q); //function to check if the queue is empty
int isQueueFull(Queue* q); //function to check if the queue is full
void enqueue(Queue* q, struct Task *task); //the function that adds a task to the queue
struct Task* dequeue(Queue* q); //the function that extracts the next task to run in the scheduler
void callInitializeQueue(); //function that calls initialize queue
void initializeQueue(Queue *q); //function to initialize queue


