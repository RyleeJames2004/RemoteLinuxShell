//Rylee Roseenlieb
//Hessa Al Ali
//Operating Systems Project
//Phase 4


//include necessary header files
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>

//include the header files of the other c files
#include "shell_Commands.h"
#include "helper_Functions.h"
#include "combinations_F.h"

//define the constant buffer size
#define MAX_BUFFER 1000



void myshell(char *command) {
    char *args[MAX_BUFFER]; //initiate array to store the commands, arguments, and symbols
    char userInput[MAX_BUFFER]; //initatiate buffer to store input from user
    strcpy(userInput, command);//command copied to user inpu

    userInput[strcspn(userInput, "\n")] = '\0'; //remove "new line" from the userInput

    //if the user input is empty, it prints an error
    if (strcmp(userInput,"") == 0){
        printf("Empty command line\n");
        return;
    }
    bool condition; // Boolean variable to check if command has <, >, or |

    

    //call the functions that will parse the input and check for compound command
    parseInput(userInput, args); //parse the input
    condition = search(args); // Check for <, >, or |

    if (strcmp(args[0], "gcc") == 0) { //check if a program was entered into the command line
        // Execute the gcc command as entered
        executeProgram(args); // Execute the command
        return;
    }
    
    //appropriately send args to the correct function
    if (condition) { //if args includes compound combination symbols
        compoundCombinations(args); //send arguments to function that handles compound commands
    } 
    else { //perform the 15 singular commands
        if (strcmp(args[0],"ls")==0){
            executeCommand(args);
        }
        else if(strcmp(args[0],"ps")==0){
            executeCommand(args);
        }
        else if (strcmp(args[0],"pwd")==0){
            executeCommand(args);
        }
        else if (strcmp(args[0],"mkdir")==0){
            executeCommand(args);
        }
        else if (strcmp(args[0],"rmdir")==0){
            executeCommand(args);
        }
        else if (strcmp(args[0],"touch")==0){
            executeCommand(args);
        }
        else if (strcmp(args[0],"rm")==0){
            executeCommand(args);
        }
        else if (strcmp(args[0],"whoami")==0){
            executeCommand(args);
        }
        else if (strcmp(args[0],"cat")==0){
            executeCommand(args);
        }
        else if (strcmp(args[0],"echo")==0){
            executeCommand(args);
        }
        else if (strcmp(args[0],"mv")==0){
            executeCommand(args);
        }
        else if (strcmp(args[0],"cp")==0){
            executeCommand(args);
        }
        else if (strcmp(args[0],"sort")==0){
            executeCommand(args);
        }
        else if (strcmp(args[0],"grep")==0){
            executeCommand(args);
        }
        else if (strcmp(args[0],"wc")==0){
            executeCommand(args);
        }
        else if (strcmp(args[0],"clear")==0){ //added for a 16th command for convenience
            executeCommand(args);
        }
        else if (strncmp("./",args[0],2)==0){ //if it is an executable (e.g. ./hello)
            executeCommand(args);
        }
        else{ //if the inputed command is not recognized, then display to the user that it was an invalid command
            printf("Invalid command : No such file or directory\n");
        }
    }     

    return;
}


