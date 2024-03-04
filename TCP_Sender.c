#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <netinet/tcp.h>

#define BUFFER_SIZE 1024
#define EXIT_FAILURE 1

char *util_generate_random_data(unsigned int size) {     
    char *buffer = NULL;      
    // Argument check.     
    if (size == 0){
        return NULL;  
    }         
       
    buffer = (char *)calloc(size, sizeof(char));      
    // Error checking.     
    if (buffer == NULL) {
        return NULL;      

    }        
    // Randomize the seed of the random number generator.     
    srand(time(NULL));      
    for (unsigned int i = 0; i < size; i++){         
        *(buffer + i) = ((unsigned int)rand() % 256);  
    }    
    return buffer; 
} 

int main(int argc, char** argv) {

    if (argc != 7) {
        fprintf(stderr, "Invalid command line - it should be: %s -p <port> -algo <algorithm>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char* RECEIVER_IP = argv[2];

    int PORT = atoi(argv[4]);
    if (PORT <= 0 || PORT > 65535) {
        fprintf(stderr, "Invalid port number: %s\n", argv[4]);
        return EXIT_FAILURE;
    }

    char *algo = argv[6];
    if (strcmp(algo, "reno") != 0 && strcmp(algo, "cubic") != 0) {
        fprintf(stderr, "Invalid algorithm: %s\n", algo);
        return 1;
    }
    socklen_t algo_len = strlen(algo);
    
    int sock = -1;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    // Create socket - if the socket creation failed, print an error message and return 1.
    if (sock == -1) {
        perror("Socket creation failed");
        return EXIT_FAILURE;
    }

    if (setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, algo, algo_len) != 0) {
        perror("setsockopt");
        close(sock);
        return EXIT_FAILURE;
    }
    struct sockaddr_in server_addr; // The variable to store the server's address.

    memset(&server_addr, 0, sizeof(server_addr)); // Reset the server structure to zeros. 
    // Server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(RECEIVER_IP);

    // Connect to server
    // Try to connect to the server using the socket and the server structure.
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        return EXIT_FAILURE;
    }    
    printf("Socket successfully created.\n");
    printf("Connected successfully to %s:%d!\n", RECEIVER_IP, PORT);

    while (1) {
        // create a random file according to the given algorythm in the appendix. the size of the file need to be 2MB.
        unsigned int f_size = 2 * 1024 * 1024; // 2MB
        char *data = util_generate_random_data(f_size);

        // Write the file
        FILE *file = fopen("file.txt", "wb");
        if (file == NULL) {
            perror("Opening failed");
            return EXIT_FAILURE; 
        }
        fwrite(data, sizeof(char), f_size, file);
        fclose(file);

        // Send the file
        file = fopen("file.txt", "rb");
        if (file == NULL) {
            perror("Opening failed");
            return EXIT_FAILURE; 
        }
        char buffer[BUFFER_SIZE];
        size_t bytes_read;

        while ((bytes_read = fread(buffer, sizeof(char), BUFFER_SIZE, file)) > 0) {
            if (send(sock, buffer, bytes_read, 0) < 0) {
                perror("Send failed");
                return EXIT_FAILURE; 
            }
        }

        fclose(file);
        free(data);   // Cleanup

        // the user needs to decide whether to send the file again or not
        char des;
        printf("Do you want to send the file again? (y/n): ");
        scanf(" %c", &des);
        if (des != 'y') {
            printf("sending an exit message - ready to exit\n");
            break; // Exit the loop
        }
    }

    close(sock);

    printf("The socket colesed- good bye ;)\n");
    
    return 0;
}