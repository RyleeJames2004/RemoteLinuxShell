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
#include <stdio.h>

int main(int argc, char *argv[]){

int n= atoi(argv[1]);// paramater passed into the test.c
for(int i=0; i<n+1; i++){
	printf("%d/%d \n",i,n);
	sleep(1);

}

return 0;

}