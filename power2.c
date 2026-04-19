#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>

int main(){
	printf("Powers of 2 from 0 to 5: \n");
	for (int i = 0; i < 5; i++){
		printf("2^%d = %d\n", i, 2^i);
	}

	return 0;
}

