

# Default target to build everything
all: myshell client demo

# Build myshell from object files
myshell: server.o scheduler.o myshell.o shell_Commands.o helper_Functions.o combinations_F.o
	gcc server.o scheduler.o myshell.o shell_Commands.o helper_Functions.o combinations_F.o -o myshell -pthread

# Build client from object files
client: client.o
	gcc client.o -o client

#build demo from object files
demo: demo.o  
	gcc demo.o -o demo

# Compile each .c file into .o
server.o: server.c scheduler.h myshell.h
	gcc -c server.c

scheduler.o: scheduler.c scheduler.h myshell.h shell_Commands.h helper_Functions.h
	gcc -c scheduler.c

myshell.o: myshell.c myshell.h shell_Commands.h helper_Functions.h combinations_F.h
	gcc -c myshell.c

shell_Commands.o: shell_Commands.c shell_Commands.h
	gcc -c shell_Commands.c

helper_Functions.o: helper_Functions.c helper_Functions.h
	gcc -c helper_Functions.c

combinations_F.o: combinations_F.c combinations_F.h
	gcc -c combinations_F.c

client.o: client.c
	gcc -c client.c

demo.o: demo.c  
	gcc -c demo.c

# Clean up object files and executables
clean:
	rm -f *.o myshell client demo
