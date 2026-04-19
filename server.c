// Rylee Rosenlieb
// Hessa Al Ali
// Operating Systems Project
// Phase 4


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

#include "scheduler.h"
#include "myshell.h"

// Define the constant buffer size
#define MAX_BUFFER 1000
#define PORT 9002
//#define PORT 8080
//#define TIME_QUANTUM 3
#define MAX 100

void* serverFunction(void* client_data);

int main() {    
    // Create the server socket
    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // Check for socket creation failure
    if (server_socket == -1) {
        printf("Socket creation failed..\n");
        exit(EXIT_FAILURE);
    } else {
        printf("Server: socket CREATION success..\n");
    }

    // Define server address structure
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET; //
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to our specified IP and port
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) { //check if there was a failure in the bind and print if so
        printf("Socket bind failed..\n"); 
        exit(EXIT_FAILURE);
    } else {
        printf("Server: socket BIND success..\n"); //print if the bind was a success
    }

    // Listen for connections, with a queue of up to 5 connections
    if (listen(server_socket, 5) < 0) { //check if there the socket is failing to listen
        printf("Listen failed..\n");
        exit(EXIT_FAILURE);
    } else {
        printf("Server: socket LISTEN success..\n"); //print if the server is successfully listening for clients
    }

    //create and start the scheduler thread
    pthread_t schedulerThread; //create the schedule thread variable
    pthread_create(&schedulerThread, NULL, scheduler, NULL); //create the scheduler thread and start the scheduler function
    pthread_detach(schedulerThread); //detach the thread

    // Accept client connections and create a thread for each
    //*create the client socket variable and structure
    int client_socket;
    struct sockaddr_in client_address;
    socklen_t addrlen = sizeof(client_address);
    int clientId = 1; //initiliaze the first client id to be 1

    //infinitely loop to accept clients
    while (1) {  
        // Accept new client connection (blocking until a client connects)
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &addrlen);
        if (client_socket < 0) { //check if there was a failure to accept a client
            printf("Accept failed..\n");
            continue;
        } else {            
            printf("[%d]<<< client connected\n", clientId); //print on the server that there has been a client connection

            // Allocate and initialize client data struct (to contain all the socket and id information of the client)
            struct ClientData* client_data = (struct ClientData*)malloc(sizeof(struct ClientData));
            client_data->socket = client_socket; //assign the client socket variable
            client_data->address = client_address; //assign the the address struct the client_address
            client_data->clientID = clientId;

            // Create a thread to handle the client
            pthread_t thread;
            pthread_create(&thread, NULL, serverFunction, (void*)client_data); //create thread client_data as an argument to the thread function: serverFunction
            pthread_detach(thread);  // Detach thread

            clientId += 1; //increase the clientID by one for the next user
        }        
    }

    // Close the server socket
    close(server_socket);
    printf("Closed server socket\n");

    return 0;
}


// Function that handles communicating with the client; receives the command and sends the output
void* serverFunction(void* client_data) {
    struct ClientData* data = (struct ClientData*)client_data; //convert the passed argument client_data back into struct ClientData
    int new_sock = data->socket; //assign the socket in the struct to variable new_sock
    struct sockaddr_in client_address = data->address; //assign the address in the struct to client address
    int clientID = data -> clientID;    
    size_t bytesInBuffer; //initialize a variable of type size_t to store the total bytes sent of a normal command

    char buffer[MAX_BUFFER];      // Buffer to store incoming messages from client
    char outputBuffer[MAX_BUFFER]; // Buffer to store the output to send to the client

    while (true) {
        // Receive a message from the client and store it into the buffer
        recv(new_sock, &buffer, MAX_BUFFER, 0);

        // Check for "exit" message to close connection
        if (strncmp("exit", buffer, 4) == 0) { //if it is exit in the buffer
            //printf("[%s:%d] Client disconnected\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port)); //print that the client is disconnecting
            printf("[%d]>>> client disconnected\n", clientID);
            close(new_sock); //close the client socket
            free(data);  // Free the client data struct
            pthread_exit(NULL); //exit the thread
        }

        buffer[strcspn(buffer, "\n")] = '\0'; //strip the new line character from the buffer
        //printf("[%s:%d] Received command: \"%s\"\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), buffer); //print what command was recieved from the client
        printf("[%d]>>> %s\n", clientID, buffer);

        int pipe_fd[2]; //create the pipe file descriptors
        if (pipe(pipe_fd) == -1) { // Create the pipe and check for errors
            perror("Pipe creation failed");
            exit(EXIT_FAILURE); //exit if there was a failure in the pipe creation
        }    

        //check if the input in the buffer starts with ./demo
        if (strncmp("./demo", buffer, 6) == 0){
            struct newExecutable* item = (struct newExecutable*)malloc(sizeof(struct newExecutable)); //create a new executable demo item
            item -> socket = new_sock; //assign the client socket in the item
            item -> address = client_address; //assign the client address in the item
            item -> clientID = clientID; //assign the client id in the item
            strncpy(item->program, buffer, MAX_BUFFER); //copy the item buffer content into the newExecutables program buffer         
            add_to_queue(item); //send the new item to the function that creates the task and executable thread
        }
        else { //otherwise it is a normal command and run as normal
            pid_t pid = fork(); // Fork a child process
            if (pid < 0) { // If fork fails
                perror("Fork failed"); //print that it failed
                exit(EXIT_FAILURE); //exit it there is a failure
            } else if (pid == 0) { // in the child process
                close(pipe_fd[0]); //  close the read end of the pipe
                printf("(%d)--- \033[38;5;33mcreated\033[0m (-1)\n", clientID); //print that the task was created
                printf("(%d)--- \033[1;32mstarted\033[0m (-1)\n", clientID); //print that the task started
                dup2(pipe_fd[1], STDOUT_FILENO); // Redirect stdout to pipe write end
                dup2(pipe_fd[1], STDERR_FILENO); // Redirect stderr to pipe write end
                myshell(buffer); 
                close(pipe_fd[1]); // Close write end of pipe
                exit(EXIT_SUCCESS);
            } else { // Parent process
                wait(NULL); // Wait for the child process to finish
                close(pipe_fd[1]); // Close write end of pipe
                bzero(outputBuffer, MAX_BUFFER); // Clear output buffer
                read(pipe_fd[0], outputBuffer, MAX_BUFFER); // Read from pipe into output buffer
                send(new_sock, outputBuffer, MAX_BUFFER, 0); // Send output buffer to client
                close(pipe_fd[0]); // Close read end of pipe                
                printf("(%d)--- \033[1;31mended\033[0m (-1)\n", clientID); //print that the command execution ended
                bytesInBuffer = strlen(outputBuffer); //get the bytes sent and store in the variable that stores the bytes           
                printf("(%d)<<< %ld bytes sent\n", clientID, bytesInBuffer); //print out the total bytes sent

            }            
        }
    }

    return NULL;
}



