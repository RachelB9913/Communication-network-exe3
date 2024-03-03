#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#define PORT 5630
#define BUFFER_SIZE 1024
#define FILE_NAME "data.txt"
#define MAX_CLIENTS 1
#define EXIT_FAILURE 1

int syncTimes(int socket, int loss);

//./TCP_Receiver -p 5630 -algo reno

int main(void) {
    // Create sockets
    int sock = -1;
    int new_sock = -1;

    // The variable to store the server's address.
    struct sockaddr_in receiver_addr;

    // The variable to store the client's address.
    struct sockaddr_in sender_addr;

    struct timeval tv_start, tv_end;

    unsigned int recv_addr_len;

    int fileSize, run_index = 0 ,totalReceived = 0, currentSize = 5;

    int opt = 1;

    char recvAddr[INET_ADDRSTRLEN];

    char* buffer = NULL;

    double *all_times = (double*) malloc(currentSize * sizeof(double));

    // Reset the server and client structures to zeros.
    memset(&receiver_addr, 0, sizeof(receiver_addr));
    memset(&sender_addr, 0, sizeof(sender_addr));

    receiver_addr.sin_family = AF_INET; //IPv4
    receiver_addr.sin_addr.s_addr = INADDR_ANY;
    receiver_addr.sin_port = htons(PORT);

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if(sock==-1){
        perror("problem in socket creation");
        return EXIT_FAILURE;
    }

    // Set the socket option to reuse the server's address.
    // This is useful to avoid the "Address already in use" error message when restarting the server.
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("Address already in use");
        close(sock);
        return EXIT_FAILURE;
    }
  
    // Bind socket
    if (bind(sock, (struct sockaddr *)&receiver_addr, sizeof(receiver_addr)) < 0) {
        perror("Bind failed");
        close(sock);
        return EXIT_FAILURE;
    }

    // Listen for connections
    if (listen(sock, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        close(sock);
        return EXIT_FAILURE;
    }

    printf("Socket successfully created.\n");
    fprintf(stdout, "Listening for incoming connections on port %d...\n", PORT);

    recv_addr_len = sizeof(receiver_addr);

    new_sock = accept(sock, (struct sockaddr *)&receiver_addr, (socklen_t *)&recv_addr_len);

    if (new_sock == -1) {
        perror("problem in accept");
        return EXIT_FAILURE;
    }

    //fprintf(stdout, "Client %s:%d connected\n", inet_ntoa(sender_addr->sin_addr), ntohs(sender_addr->sin_port));

    inet_ntop(AF_INET, &(receiver_addr.sin_addr), recvAddr, INET_ADDRSTRLEN);

    printf("Connection made with {%s:%d}.\n", recvAddr, receiver_addr.sin_port);
    
    printf("Getting file size...\n");

    //getDataFromClient(new_sock, &fileSize, sizeof(int));
    int recvbytes = recv(new_sock, &fileSize, sizeof(int), 0);
    if (recvbytes == -1)
    {
        perror("recv");
        return EXIT_FAILURE;
    }

    else if (!recvbytes)
    {
        printf("Connection with client closed.\n");
        return 0;
    }

    syncTimes(new_sock, 1);

    printf("Expected file size is %d bytes.\n", fileSize);

    buffer = malloc(fileSize * sizeof(char));

    if (buffer == NULL)
    {
        perror("malloc");
        return EXIT_FAILURE;
    }

    while (1) {

        //ssize_t valread;
        if (!totalReceived){
            printf("Waiting for sender data...\n");
            gettimeofday(&tv_start, NULL);
        }
        
        int BytesReceived = recv(new_sock, buffer, fileSize-totalReceived, 0);
        if (BytesReceived == -1)
        {
            perror("recv");
            return EXIT_FAILURE;
        }

        else if (!BytesReceived)
        {
            printf("Connection with client closed.\n");
            return 0;
        }

        totalReceived += BytesReceived;

        if (!BytesReceived){
            break;
        }
        
        else if (totalReceived == fileSize){
            gettimeofday(&tv_end, NULL);
            all_times[run_index] = ((tv_end.tv_sec - tv_start.tv_sec)*1000) + (((double)(tv_end.tv_usec - tv_start.tv_usec))/1000); //calculate the time
            
            run_index++;
            if (run_index >= currentSize){
                currentSize += 5;
                double* times = (double*) realloc(all_times, (currentSize * sizeof(double)));
                if (times == NULL){
                    perror("realloc");
                    return EXIT_FAILURE;
                }
                all_times=times;
            }
            printf("Received total %d bytes.\n", totalReceived);
            syncTimes(new_sock, 1);
            printf("Waiting for sender decision...\n");
            if (!syncTimes(new_sock, 0)){
                break;
            }
            syncTimes(new_sock, 1);
            //totalReceived = 0;
        }
    }
 //------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // Close sockets
    close(new_sock);
    double sum_times=0, avg=0;
    printf("Connection with {%s:%d} closed.\n", recvAddr, receiver_addr.sin_port);
    printf("----------------------------------\n");
    printf("- * Statistics * -\n");
    for(int i = 0; i < run_index; ++i)
    {
        sum_times += all_times[i];
        printf("(*) Run %d, Time: %0.3lf ms\n", (i+1), all_times[i]);
    }
    if (run_index > 0)
    {
        avg = (sum_times / (double)run_index);
    }
    double bandwidth = ((fileSize / 1024) / (avg / 1000));
    printf("- Average time: %.1fms\n", avg);
    printf("- Average bandwidth: %.6fMB/s\n", bandwidth);
    printf("----------------------------------\n");
    
    printf("Memory cleanup...\n");
    free(all_times);
    free(buffer);
    printf("Closing socket...\n");
    close(sock);
    printf("Receiver end.\n");
    return 0;
}

//
int syncTimes(int socket, int loss) {
    static char x = 0;
    static int ans = 0;

    if (loss)
    {
        ans = send(socket, &x, sizeof(char), 0);

        if (ans == -1){
            perror("send");
            return EXIT_FAILURE;
        }
        return ans;
    }

    printf("Waiting for signal from sender...\n");
    ans = recv(socket, &x, sizeof(char), 0);

    if (ans == -1)
    {
        perror("recv");
        return EXIT_FAILURE;
    }

    printf("Continue...\n");

    return ans;
}