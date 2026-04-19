//Rylee Roseenlieb
//Hessa Al Ali
//Operating Systems Project
//Phase 4

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>

#include <netinet/in.h>


//define the constant buffer size
#define MAX_BUFFER 1000

#define PORT 9002
//#define PORT 8080

//declare any functions
void clientFunction(int network_socket);

int main(){

	//create a socket
	//use an int to hold the fd for the socket
	int network_socket;

	//1st argument: domain/family of socket. For Internet family of IPv4 addresses, we use AF_INET 
	//2nd: type of socket TCP
	//3rd: protocol for connection, left 0 when using tcp
	network_socket = socket(AF_INET , SOCK_STREAM, 0);

	//check for fail error
	if (network_socket == -1) {
        printf("socket creation failed..\n"); //print if there was an error
        exit(EXIT_FAILURE);
    }
    else
    	printf("Client: socket CREATION success..\n"); //print if the client socket creation 
    

	//connect to another socket on the other side
	
	//specify an address for the socket we want to connect to
	struct sockaddr_in server_address;
	
	//specify address family
	server_address.sin_family = AF_INET;
	
	//specify the port we want to connect to remotely
	//htons converts integer port to right format for the structure	
	server_address.sin_port = htons(PORT);

	//specify the actual IP address
	//connect to our local machine
	//INADDR_ANY gets any IP address used on our local machine
	//INADDR_ANY is an IP address that is used when we don't want to bind a socket to any specific IP. Basically, while implementing communication, we need to bind our socket to an IP address. When we don't know the IP address of our machine, we can use the special IP address INADDR_ANY. It allows our server to receive packets that have been targeted by any of the interfaces.
	server_address.sin_addr.s_addr = INADDR_ANY;


	//connect
	//1st : socket 
	//2nd: server address structure , cast to a pointer to a sockaddr struct so pass the address
	//3rd: size of address
	int connection_status = 
	connect(network_socket, 
			(struct sockaddr *) &server_address,
			sizeof(server_address));

	//check for errors with the connection
	if(connection_status == -1){
		printf("There was an error making a connection to the remote socket \n\n"); //print if unable to connect to server
		exit(EXIT_FAILURE);
	}
	else
    	printf("Client: socket CONNECT success..\n"); //print that the client connected to the server

    clientFunction(network_socket); //call function to perform the client functionalities; communicate with server

	//close socket after we are done
	close(network_socket);

	printf("Closed client socket\n"); //print a confirming statement that the client socket closed successfully



	return 0;
}

//function to handle the client terminal and communication with the server
void clientFunction(int network_socket){
	char buffer[MAX_BUFFER]; //initatiate buffer to store input from user
	while (true){ //loop until condition set to exit loop
        printf("myshell ~ %% "); //print the shell line
        fgets(buffer, MAX_BUFFER, stdin); //get the input from the user

 		if (strncmp(buffer, "exit",4) == 0) { //if user input is exit
			printf("Client Exiting...\n"); //print that the client is exiting the connection
			send(network_socket, buffer,MAX_BUFFER,0); //send to the server that the client is exiting
			return; //return to the main function
		}

		if (strncmp(buffer, "./demo", 6) != 0){  //find if the input buffer is a just a command
	        send(network_socket, buffer,MAX_BUFFER,0); //send the input in the buffer to the server	        
			recv(network_socket , &buffer , MAX_BUFFER,0); //recieve the results from the server and store in  buffer			
			printf("%s\n", buffer); //print the buffer
		}
		else{ //otherwise, if it is a ./demo executable
			send(network_socket, buffer,MAX_BUFFER,0); //send the input in the buffer to the server
			while (1){ //enter a loop to print out strings from the buffer
				recv(network_socket , &buffer , MAX_BUFFER,0); //recieve the results from the server and store in  buffer
				if (strncmp(buffer,"done", MAX_BUFFER)== 0){ //first check if the message receive is "done"
					break; //if the message is "done", then this means there is no more items being sent to the client so exit the loop
				}
				printf("%s", buffer); //print the buffer
			}
		}
	}
}