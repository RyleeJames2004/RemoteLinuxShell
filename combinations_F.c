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

#define MAX_BUFFER 200




//Handle commands with pipes or redirection
 void compoundCombinations(char **args) {    
	int input_fd = 0;  // initial file descripter for inputs, which is set to STDIN (0). This will change if there is redirection input
    int output_fd = 1; // initial file descripter for outputs, which is set to STDOUT (1). This will change if there is redirection output
    int pipefd[2];     // The pipe file descripters to create the pipe for when there is "|"
    int i = 0; //create variable to iterate over arguments array 
    bool error = false; //condition to signify if there is error redirection
    int error_fd = 1; //initial file descriptor for error redirection
    int standard_error_fd = -1; //variable to save the file descriptor for error output when 
                                //there is error redirection


    //add command arguments until a special symbol is encountered
    while (args[i] != NULL) { //while there are strings in the args array
        char *cmd_args[MAX_BUFFER];  //create a buffer to store the commands themselves in //try outside loop and clear it
        int arg_count = 0;   //variable to keep track of the commands being added to the cmd_args array

        //check if the value in the very first index of the array is not a command
        if (strcmp(args[0], "|") == 0 || strcmp(args[0], "<") == 0 || strcmp(args[0], ">") == 0 || strcmp(args[0], "2>") == 0){
            printf("Missing command\n"); //print that it was a missing command
            return; 
        }

        // Collect command arguments until a special symbol is encountered
        while (args[i] != NULL && strcmp(args[i], "|") != 0 && strcmp(args[i], "<") != 0 && strcmp(args[i], ">") != 0 && strcmp(args[i], "2>") != 0) {
            //if not encountered special symbol or null, add it to the array
            //increment number of arguments in command array
            //increment i to get next string in args
            cmd_args[arg_count] = args[i];
            arg_count += 1;
            i += 1;            
        }
        cmd_args[arg_count] = NULL; //add NULL to signify that the end of the listed commands in the array

        // input redirection (<)
        if (args[i] != NULL && strcmp(args[i], "<") == 0) {
            i++; //go to the next index in the array
            if (args[i] == NULL || strcmp(args[i], "|") == 0 || strcmp(args[i], "<") == 0 || strcmp(args[i], ">") == 0 || strcmp(args[i], "2>") == 0 ){
                printf("Input file not specified\n"); //print that there is a missing file and exit
                return;
            }
            input_fd = open(args[i], O_RDONLY); //open arg[i] (a text file) and redirect as input for the command of arg[i-2]; can only read from the text file
            //input_fd becomes the file descriptor of the text file

            if (input_fd < 0) { //if the file in arg[i] doesn't exist, print an error and exit
                perror("Failed to open input file"); 
                i++; //increase the i index counter
                while (args[i] != NULL) { //check that there wasn't an error redirection statement later in the command
                    if (strcmp(args[i], "2>") == 0) { //if there is, then check that there is an error file in the index after
                        i++;
                        if (args[i] != NULL) { //if there is an error file: create it if it doesn't exist or clear it out if it does
                            error_fd = open(args[i], O_WRONLY | O_CREAT | O_TRUNC, 0644); // Create or clear error.log
                            if (error_fd < 0) {
                                exit(EXIT_FAILURE); //if there was a failure in creating or opening, exit 
                            }
                        }
                        break; //break out of the while loop
                    }
                    i++;
                }
                return;

            }
            i++; //iterate in args
        }

        //output redirection (>)
        if (args[i] != NULL && strcmp(args[i], ">") == 0) {
            i++; //go to the next index in the array

            //check if the value in the next index of args is not a command
            if (args[i] == NULL || strcmp(args[i], "|") == 0 || strcmp(args[i], "<") == 0 || strcmp(args[i], ">") == 0 || strcmp(args[i], "2>") == 0 ){
                printf("Output file not specified\n"); //print that there is a missing file and exit
                return;
            }
            output_fd = open(args[i], O_WRONLY | O_CREAT | O_TRUNC, 0644); //output is written to the file; if file doesn't exist, create it
            //output_fd becomes the file descriptor of the text file

            if (output_fd < 0) { //check if there was an issue in opening or creating the file
                perror("Failed to open output file"); //print an error message and exit if there was an issue                
                exit(EXIT_FAILURE);
            }            
            i++; //iterate in args
        }

        //handle error redirection
        if (args[i] != NULL && strcmp(args[i], "2>") == 0) {
            i++; //go to the next index in the array

            //check if the value in the next index of args is not a command
            if (args[i] ==NULL || strcmp(args[i], "|") == 0 || strcmp(args[i], "<") == 0 || strcmp(args[i], ">") == 0 || strcmp(args[i], "2>") == 0 ){
                printf("error output file not specified\n"); //print that there is a missing command and exit
                return;
            }
            
            error_fd = open(args[i], O_WRONLY | O_CREAT | O_TRUNC, 0644); //error is written to the file; if file doesn't exist, create it
            if (error_fd < 0) { //check if there was an issue in opening or creating the file
                perror("Failed to open error file"); //print an error message and exit if there was an issue                
                exit(EXIT_FAILURE);
            }

            standard_error_fd = dup(STDERR_FILENO); //save current file descriptor for error output

            dup2(error_fd, STDERR_FILENO); // Redirect stderr to file descriptor in error_fd
            close(error_fd); // Close the file after redirect
            i++; //iterate in args
            error = true;
        }

        // Handle piping (|)
        if (args[i] != NULL && strcmp(args[i], "|") == 0) {
            int j = i + 1; //index for next index in args

             //check if the value in the next index of args is not a command
            if (args[j] == NULL || strcmp(args[j], "|") == 0 || strcmp(args[j], "<") == 0 || strcmp(args[j], ">") == 0 || strcmp(args[j], "2>") == 0 ){
                printf("Missing command after pipe\n"); //print that there is a missing command and exit
                return;
            }
            pipe(pipefd); // Create a pipe
            pid_t pid = fork(); // Fork a child

            if (pid < 0) { //check if fork was successful
                perror("Error during fork."); //print error and exit if unsuccessful
                exit(EXIT_FAILURE); 

            }
            else if (pid == 0) { // Child process
                dup2(input_fd, STDIN_FILENO);     // Redirect stdin to input_fd
                dup2(pipefd[1], STDOUT_FILENO);   // Redirect stdout to pipe write end
                
                close(pipefd[0]);                 // Close read end of the pipe  
                
                // Execute the command              
                if (execvp(cmd_args[0], cmd_args)== -1){ //execute the commands and check if it was successful
                    perror("Invalid command\n"); // if unsuccessful, print error statement and exit
                    exit(EXIT_FAILURE);
                }   
            } 
            else { // Parent process
                wait(NULL);        // Wait for child process
                close(pipefd[1]);  // Close write end of the pipe
                input_fd = pipefd[0]; // read the output from the pipe and put it into the input_fd file descriptor
                i++; //iterate in args
            }
        } else { // Last command or no pipe
            pid_t pid = fork(); // Fork a child
            if (pid < 0) { //check if fork was successful
                perror("Error during fork"); //print error and exit if unsuccessful
                exit(EXIT_FAILURE);
            }
            else if (pid == 0) { // Child process
                dup2(input_fd, STDIN_FILENO); //redirect to read from the input_fd and not STDIN                
                close(input_fd); //close the file (descriptor)

                if (output_fd != 1) {         //if output_fd file descriptor value was changed from 1, indicating redirection
                    dup2(output_fd, STDOUT_FILENO);  //read direct output to file at file descriptor in output_fd
                }  

                if (execvp(cmd_args[0], cmd_args) == -1){ // Execute the final command
                    perror("Invalid command"); //Execute the final command or the redirection command
                    exit(EXIT_FAILURE);                    
                }
                close(output_fd); //close the file (descriptor) in output_fd
                if (error == true){ //if there was error redirection
                    close(error_fd); //close the error file (descriptor)
                    error = false; //reset boolean expression
                }
            } else { // Parent process
                wait(NULL); //wait for the child to finish  
                if (standard_error_fd != -1){ //if error output was redirected, change it back to standard error output
                    dup2(standard_error_fd, STDERR_FILENO);
                    close(standard_error_fd);
                    standard_error_fd = -1; //reset
                }              
            }
        }
    }
}

