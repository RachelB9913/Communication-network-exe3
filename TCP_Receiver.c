#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/tcp.h>

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 1
#define EXIT_FAILURE 1

//./TCP_Receiver -p 5630 -algo reno

int main(int argc, char **argv) {

    if (argc != 5) {
        fprintf(stderr, "Usage: %s <port> <algorithm>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int PORT = atoi(argv[2]);
    if (PORT <= 0 || PORT > 65535) {
        fprintf(stderr, "Invalid port number: %s\n", argv[2]);
        return EXIT_FAILURE;
    }

    char *algo = argv[4];
    if (strcmp(algo, "cubic") != 0 && strcmp(algo, "reno") != 0) {
        fprintf(stderr, "Invalid algorithm: %s\n", algo);
        return EXIT_FAILURE;
    }
    socklen_t algo_len = strlen(algo) + 1;

    // Create sockets
    int sock = -1;
    int new_sock = -1;

    struct sockaddr_in receiver_addr;    // The variable to store the server's address.
    struct sockaddr_in sender_addr;    // The variable to store the client's address.

    struct timeval tv_start, tv_end;

    unsigned int totalReceived = 0;
    unsigned int recv_addr_len;
    double total_t_recv;


    // Reset the server and client structures to zeros.
    memset(&receiver_addr, 0, sizeof(receiver_addr));
    memset(&sender_addr, 0, sizeof(sender_addr));

    printf("Starting Receiver...\n");

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if(sock==-1){
        perror("problem in socket creation");
        return EXIT_FAILURE;
    }

    // Set the socket option to reuse the server's address.
    // This is useful to avoid the "Address already in use" error message when restarting the server.
    if (setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, algo, algo_len) != 0)
    {
        perror("Address already in use");
        close(sock);
        return EXIT_FAILURE;
    }

    receiver_addr.sin_family = AF_INET; //IPv4
    receiver_addr.sin_addr.s_addr = INADDR_ANY;
    receiver_addr.sin_port = htons(PORT);

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
    fprintf(stdout, "Listening for incoming TCP connections on port %d...\n", PORT);
    
    int total = 0;
    int num_run = 1;

    while(1){
       new_sock = accept(sock, (struct sockaddr *)&receiver_addr, (socklen_t *)&recv_addr_len);

        if (new_sock < 0) {
            perror("problem in accept");
            close(sock);
            return EXIT_FAILURE;
        } 
        printf("Connection created..\n");
        printf("\n");

        gettimeofday(&tv_start, NULL);

        FILE *file = fopen("the_file.txt", "wb");
        if (file == NULL) {
            perror("file opening failed");
            return EXIT_FAILURE;
        }

        total_t_recv = 0;
        char buffer[BUFFER_SIZE];
        int bytes_recv = 1;
        unsigned int expected_bytes = 1<<21;

        while(bytes_recv){
            bytes_recv = recv(new_sock, buffer, BUFFER_SIZE, 0);
            if(bytes_recv < 0){
                perror("recv");
                return EXIT_FAILURE;
            } 
            else if (bytes_recv == 0) {
                break; // Socket closed by sender
            }

            totalReceived += bytes_recv;
            total += bytes_recv;

            if(totalReceived >= expected_bytes){
                gettimeofday(&tv_end, NULL);
                double elapsed_time = ((tv_end.tv_sec - tv_start.tv_sec) * 1000.0) + ((tv_end.tv_usec - tv_start.tv_usec) / 1000.0);
                double bandwidth = (totalReceived / (1024 * 1024)) / (elapsed_time / 1000); // Convert bytes/ms to MB/s

                // Print statistics and reset for the next run
                printf("Run #%d\n", num_run);
                printf("file transfer completed\n");
                printf("Data: Time= %.2fms    Speed= %.2fMB/s\n", elapsed_time, bandwidth);
                printf("\n");

                total_t_recv += elapsed_time;
                num_run++;
                printf("Waiting for Sender response...\n");
                totalReceived = 0; // Reset for the next run
                gettimeofday(&tv_start, NULL);
            }
        }

        fclose(file);
        close(new_sock);

        if (bytes_recv == 0) {
            printf("the sender decided to close the connection...\n");
            printf("\n");
            break; // Exit the loop
        } 

        else if (bytes_recv < 0) {
            perror("recv");
            return EXIT_FAILURE;
        }
    }

    double average_time = total_t_recv / (num_run - 1); // computes the average time
    double total_bandwidth = (total / (1024 * 1024)) / (total_t_recv / 1000); // computes the bandwidth and convert bytes/ms to MB/s
    
    printf("----------------------------------\n");
    printf("-----------Statistics-------------\n");
    printf("Total runs: %d\n", num_run-1);
    printf("Total time: %f\n", total_t_recv);
    printf("Average time: %.2fms\n", average_time);
    printf("Average bandwidth: %.2fMB/s\n", total_bandwidth);
    printf("----------------------------------\n");
    printf("The socket closed- Good bye ;)\n");
    
    return 0;
}