#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

#define PORT 5630
#define BUFFER_SIZE 1024
#define FILE_NAME "data.txt" // Assume a file named data.txt to be sent
#define EXIT_FAILURE 1
#define RECEIVER_IP "127.0.0.1"

int syncTimes(int socket, int loss);

char * fileName = "data.txt";

int main(void) {
    int sock = -1, fileSize = 0;
    char *content = NULL;
    struct sockaddr_in server_addr; // The variable to store the server's address.

    // Read the file
    FILE *file = fopen(FILE_NAME, "rb");
    if (file == NULL) {
        perror("File opening failed");
        return EXIT_FAILURE;
    }
    fseek(file, 0L, SEEK_END);
    fileSize = (int) ftell(file);
    content = (char*) malloc(fileSize * sizeof(char));
    fseek(file, 0L, SEEK_SET);

    fread(content, sizeof(char), fileSize, file);
    fclose(file);

    printf("total size is %d bytes.\n", fileSize);

    // Reset the server structure to zeros. 
    memset(&server_addr, 0, sizeof(server_addr));
    // Server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, RECEIVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid address / Address not supported");
        close(sock);
        return EXIT_FAILURE;
    }
    // Create socket - if the socket creation failed, print an error message and return 1.
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Socket creation failed");
        return EXIT_FAILURE;
    }
    printf("Socket successfully created.\n");
    
    printf("Connected successfully to %s:%d!\n", RECEIVER_IP, PORT);

    //fprintf(stdout, "Connecting to %s:%d...\n", RECEIVER_IP, PORT);

    // Connect to server
    // Try to connect to the server using the socket and the server structure.
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        return EXIT_FAILURE;
    }

    printf("Successfully connected to the server!\n"
            "Sending the file to the server\n");

    // Send file
    //size_t bytes_read = fread(buffer, 1, BUFFER_SIZE, file);
    
    //while (bytes_read != 0) {  //???????????

    int sent_file, len=sizeof(int);
    sent_file= send(sock, &fileSize, sizeof(int), 0);
    if (sent_file == -1) {
        perror("Send failed");
        close(sock);
        return EXIT_FAILURE;
    }
    else if (sent_file==0)
        printf("Receiver doesn't accept requests.\n");

    else if (sent_file < len)
        printf("some of the data wasn't sent\n");

    else
        printf("Total bytes sent is %d.\n", sent_file);


    //printf("File sent successfully\n");
    syncTimes(sock, 0);

    while(1){
        char decision;
        printf("Sending the file...\n");
        int data;
        data= send(sock, content, fileSize, 0);
        if (data < 0) {
            perror("Send failed");
            close(sock);
            return EXIT_FAILURE;
        }
        else if (data==0)
            printf("Receiver doesn't accept requests.\n");

        else if (data < fileSize)
            printf("some of the data wasn't sent\n");

        else
            printf("Total bytes sent is %d.\n", data);


        //printf("File sent successfully\n");
        syncTimes(sock, 0);
        printf("Do you want to send the file again? (y/n): ");
        scanf(" %c", &decision);
        syncTimes(sock, 1);
        syncTimes(sock, 0);
        if (decision == 'n') {
            printf("Exiting...\n");
            break;
        }
        fseek(file, 0L, SEEK_SET);        // Reset file pointer to the beginning of the file
        fread(content, sizeof(char), fileSize, file);        // Read the file content again
    }
    printf("Closing connection...\n");

    close(sock);

    printf("Memory cleanup...\n");
    free(content);

    printf("Sender exit.\n");

    return 0;

}


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

    printf("Waiting for signal from receiver...\n");
    ans = recv(socket, &x, sizeof(char), 0);

    if (ans == -1)
    {
        perror("recv");
        return EXIT_FAILURE;
    }

    printf("Continue...\n");

    return ans;
}