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


// Function declarations (prototypes)
void parseInput(char *input, char **args); 
bool search(char **args);


// Parse the input into arguments
void parseInput(char *input, char **args) {
    char *token; //variable to temporarily store command, argument, or symbol in when parsing
    char *saveptr;  // Pointer to keep track of position in the string

    int i = 0; //create varible to iterate over args
    token = strtok_r(input, " ", &saveptr);  // Initial call to strtok_r()
    while (token != NULL) { //loop until there is no more of the input to parse
        args[i] = token;  // Assign token to args[i]
        i++;
        token = strtok_r(NULL, " ", &saveptr);  //call strtok_r to parse string and return parsed part to token
    }
    args[i] = NULL;  // Null terminate the args array

    return;
}

// Search for special symbols (<, >, |) in the arguments
bool search(char **args) {
    int i = 0;  //create varible to iterate over args
    while (args[i] != NULL) {
        //check for a compound combination symbol, if there is then return true
        if (strcmp(args[i], "<") == 0 || strcmp(args[i], ">") == 0 || strcmp(args[i], "|") == 0 || strcmp(args[i],"2>")==0) {
            return true;
        }
        i++;
    }
    return false; //if no symbol was found, then return false. 
}