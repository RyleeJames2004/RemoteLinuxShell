//Rylee Roseenlieb
//Hessa Al Ali
//Operating Systems Project
//Phase 3


#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>

#define MAX_BUFFER 1000

//function to execute the 15 commands, and the executable file 
void executeCommand(char **args){
    pid_t pid = fork(); //fork a child
    if (pid < 0){ //check if fork was unsuccessful and print statement and exit if so
        perror("Fork Failed");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0){
        if (execvp(args[0],args)== -1){ //execute the command and check if it was successful
            perror("Invalid Command");
            exit(EXIT_FAILURE);
        }
    }
    else{
        wait(NULL); //parent process waits for child
    }
}

//function to compile and execute a program
void executeProgram(char **args){
    pid_t pid = fork(); //fork a child
    if (pid < 0){ //check if the fork failed; print statement if is and exit
        perror("Fork Failed");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0) {
        // In the child process
       if(execvp(args[0], args)==-1){ // Execute gcc (and -o [executable name] if provided); check if execvp() failed
            perror("execvp failed"); // If execvp fails, print error and exit
            exit(EXIT_FAILURE); 
        }
    } 
    else {
        // In the parent process, wait for the child to complete
        //wait for the child and store the status upon which the child terminates
        int status;
        wait(&status);  // Wait for the compilation to finish and store the termination status of child in "status"

        // Check if the child process (compilation) was successful
        if (WEXITSTATUS(status) == 0) { //if the child exited normally and successfully
            char executable[MAX_BUFFER] = "./a.out";  //create a default executable file (./a.out), create and store in buffer

            for (int i = 1; args[i] != NULL; i++) { //loop through starting at index 1 to find if there is a -o
                if (strcmp(args[i], "-o") == 0 && args[i + 1] != NULL) { //if -o idenitified, ensure there is a name specified in the following index
                    strcpy(executable, "./"); //store ./ in the executable
                    // Then, concatenate the specified executable name to "./"
                    strcat(executable, args[i + 1]);
                    break; //exit out of the loop upon identifying
                }
            }

            // Execute the compiled binary
            pid_t execPid = fork(); //fork another child
            if (execPid < 0) { //if the fork failed, print a failure statment and exit
                perror("Fork Failed");
                exit(EXIT_FAILURE);
            }
            else if (execPid == 0) { //in the child
                char *execArgs[] = {executable, NULL};  //create an array with the executable buffer to pass an argument to execvp()
                if (execvp(execArgs[0], execArgs)==-1){  //run the executable file and check if the execution failed
                    perror("execvp failed"); //upon failure, print out a failure statement and exit
                    exit(EXIT_FAILURE);
                }
            } 
            else{
                wait(NULL);  //wait for child to finish 
            }
        }
    }
}