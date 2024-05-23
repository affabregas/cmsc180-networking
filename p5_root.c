#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <syscall.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define NUMBER_OF_THREADS 11
// #define NO_OF_CLI 4;
#define SERVER_IP "127.0.0.1"

int main(int argc, char **argv) {
    int NumberOfInputs = atoi(argv[1]);
    int PORT_NUMBER = atoi(argv[2]);
    int NUMBER_OF_CLI = atoi(argv[4]);

    printf("NO of cli: %d\n",NUMBER_OF_CLI);

    int client_size;
    int no_of_conn = 0;
    int client_sockets[NUMBER_OF_CLI];
    struct sockaddr_in client_addresses[NUMBER_OF_CLI];
    socklen_t client_address_len;

    client_sockets[0] = socket(AF_INET, SOCK_STREAM, 0);

    client_addresses[0].sin_family = AF_INET;
    client_addresses[0].sin_port = htons(PORT_NUMBER);
    client_addresses[0].sin_addr.s_addr = inet_addr(SERVER_IP);

    if(bind(client_sockets[0], (struct sockaddr*)&(client_addresses[0]), sizeof(client_addresses[0]))<0){
        printf("Couldn't bind to the port\n");
        return -1;
    }

    if(listen(client_sockets[0], 1) < 0){
        printf("Error while listening\n");
        return -1;
    }

    printf("\nListening for incoming connections.....\n");

    client_size = sizeof(client_addresses[0]);
    for (int i = 1; i < NUMBER_OF_CLI; i++) {
        client_sockets[i] = 0;
    }

    while (1) {
        int new_socket = accept(client_sockets[0], (struct sockaddr*)&client_addresses[1 + no_of_conn], (socklen_t *)&client_size);
        if (new_socket < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }
        printf("New client connected\n");

        // Add new client to array
        for (int i = 1; i < NUMBER_OF_CLI; i++) {
            if (client_sockets[i] == 0) {
                client_sockets[i] = new_socket;
                break;
            }
        }

        // If too many clients, reject connection
       

        no_of_conn++;

         if (no_of_conn == NUMBER_OF_CLI) {
            printf("Client quota reached.\n");
            // close(new_socket);
            break;
        }
    }

    


}
