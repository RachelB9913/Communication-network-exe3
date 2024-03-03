CC = gcc
FLAGS = -Wall -Wextra -Werror -std=c99 -pedantic
RM = rm -f

# Phony targets - targets that are not files but commands to be executed by make.
.PHONY: all default clean run_tcp_server runtc runts runuc runus

# Default target - compile everything and create the executables and libraries.
all: TCP_Sender TCP_Receiver 

default: all

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

# Remove all the object files
clean:
	$(RM) *.o TCP_Sender TCP_Receiver

# # Run tcp sender.
# runts: TCP_Sender
# 	./TCP_Sender

# # Run tcp client.
# runtc: TCP_Receiver
# 	./TCP_Receiver

# # Run the tcp server with system trace.
# runts_trace: TCP_Sender
# 	strace ./TCP_Sender

# # Run the tcp client with system trace.
# runtc_trace: TCP_Receiver
# 	strace ./TCP_Receiver

# # Compile all the C files into object files.
# %.o: %.c
# 	$(CC) $(CFLAGS) -c $< -o $@


