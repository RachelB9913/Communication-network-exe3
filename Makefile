CC = gcc
FLAGS = -g -Wall
RM = rm -f

all: TCP_Sender TCP_Receiver 

TCP_Sender.o: TCP_Sender.c 
	$(CC) $(FLAGS) -c -o TCP_Sender.o TCP_Sender.c

TCP_Receiver.o: TCP_Receiver.c 
	$(CC) $(FLAGS) -c -o TCP_Receiver.o TCP_Receiver.c

# Compile the tcp server.
TCP_Sender: TCP_Sender.o
	$(CC) $(CFLAGS) -o $@ $^

# Compile the tcp client.
TCP_Receiver: TCP_Receiver.o
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: all

# Remove all the object files
clean:
	$(RM) *.o TCP_Sender TCP_Receiver
