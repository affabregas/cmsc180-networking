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

#define NUMBER_OF_THREADS 2
#define NO_OF_CLI 1;
#define SERVER_IP "10.0.2.4"


void display_table(char **x, int n, int c, double* y) {
    for(int i = 0; i < c; i++) {
        for(int j = 0; j < n; j++) {
            printf("%d ",x[i][j]);
        }

        printf("\n");
    }
    printf("\n---y vec---\n");
        for(int i = 0 ; i < n ; i ++) printf("%lf ",y[i]);
    printf("\n-------\n");
}

typedef struct matrix_packet {
    int id;
    int no_of_cols;
    char* dataCol;
    double* yDataCol;
} matrix_packet;

typedef struct thread_args{
	int id;
    int no_of_cols;
    int no_of_inputs;
    int no_of_clients;
    double* result;
    int client_socket;
    double* rVec;
    double* yVec;
    char** xVec;
} thread_args;

typedef struct arg{
	char **x;
    double *y;
    int n;
    int c;
    double *r;
    int start;
    int end;
}arg;


int set_affinity(int core_id) {
   int num_cores = sysconf(_SC_NPROCESSORS_ONLN);

   cpu_set_t cpuset;
   CPU_ZERO(&cpuset);
   CPU_SET(core_id, &cpuset);

   pthread_t current_thread = pthread_self();    
   return pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
}


void * bounded_pearson_cor(void *args) {
    arg* a = (arg *) args;


    char **x = a->x;
    double *y = a->y;
    int n = a->n;
    int c = a->c;
    double *r = a->r;
    int start = a->start;
    int end = a->end;

    int num_of_cores = sysconf(_SC_NPROCESSORS_ONLN);
    int core_id = ((end+1) / (end-start+1)) % (num_of_cores-1);
    set_affinity(core_id);
    // printf("Using core: %d\n", core_id);

    // printf("Thread created. Bounds: [%d, %d]\n", start, end);

    for(int i = start; i < end; i++) { // * O(n)

        double sum_x = 0;        // + O(1)
        double sum_y = 0;        // + O(1)
        double sum_x2 = 0;       // + O(1)
        double sum_y2 = 0;       // + O(1)
        double sum_xy = 0;       // + O(1)

        for(int j = 0; j < n; j++) {    // * O(n) 
            sum_x += x[i][j];               // + O(1)
            sum_y += (y[j]);                // + O(1)
            sum_x2 += (x[i][j] * x[i][j]);  // + O(1)
            sum_y2 += (y[j] * y[j]);        // + O(1)
            sum_xy += (x[i][j] * y[j]);     // + O(1)
        }
        
        r[i] = ((n * sum_xy) - sum_x * sum_y) / sqrt((n * sum_x2 - (sum_x*sum_x)) * (n * sum_y2 - (sum_y*sum_y)));
  
                                   
    }


    pthread_exit(NULL);

}

void pearson_cor(char **x, double *y, int n, double*r, int numberOfThreads, arg* argHolder, pthread_t* tid) {

    // Thread creation
    for(int i = 0; i < numberOfThreads; i++) {
        pthread_create(&tid[i], NULL, bounded_pearson_cor, (void *) &argHolder[i]);
    }

    // Thread joining
	for(int i=0; i < numberOfThreads; i++){
		pthread_join(tid[i], NULL);
	}


}


void *recieve_client(void *arg) {
    thread_args* a = (thread_args *) arg;

    if (send(a->client_socket, &a->id, sizeof(a->id), 0) < 0){
            printf("Can't send [ID]\n");
            return NULL;
    }

    if (send(a->client_socket, &a->no_of_cols, sizeof(a->no_of_cols), 0) < 0){
        printf("Can't send [noOfCols]\n");
    }



    // this will be loop

    int start = a->id*(a->no_of_inputs / a->no_of_clients);
    int end = (a->id + 1) * (a->no_of_inputs/a->no_of_clients);
    int status = 1;
    
    printf("is this the same: s: %d, e: %d, %d %d\n", start, end, (end-start), a->no_of_cols);
    if (a->id == a->no_of_clients-1) end = a->no_of_inputs; 
    printf("recieve client bounds: %d %d\n", start, end);
    
    
    for(int i = start ; i < end; i++) {
    	// printf("Attempting to send xVec %d\n",i);
        if (send(a->client_socket, a->xVec[i], sizeof(char) * a->no_of_inputs, 0) < 0){
            printf("Can't send [xVec]\n");
            return NULL;
        }
    	// printf("Waiting for ack from xVec %d\n",i);
        //wait for ack
        recv(a->client_socket, &status, sizeof(int), 0);
        // printf("Ack recieved from xVec %d\n",i);
        // sleep(1);
    }
    




    if (send(a->client_socket, a->yVec, sizeof(double) * a->no_of_inputs, 0) < 0){
        printf("Can't send [yVec]\n");
        return NULL;
    }

    if (recv(a->client_socket, a->result, sizeof(double) * a->no_of_cols, 0) < 0) {
        printf("Error in reading client\n");
        return NULL;
    }
    int base_offset = a->no_of_inputs / a->no_of_clients;
    for(int i = 0; i < a->no_of_cols; i++) {
    //    printf("r (%d): %lf\n", a->id, a->result[i]);     DEBUG
        a->rVec[a->id*base_offset + i] = a->result[i];   
    }

    close(a->client_socket);
    return NULL;
}





int main(int argc, char **argv) {
    int NumberOfInputs = atoi(argv[1]);
    int PORT_NUMBER = atoi(argv[2]);
    int INSTANCE_STATUS = atoi(argv[3]);

    int NUMBER_OF_CLI = NO_OF_CLI;








    
    // Server Code
    if (INSTANCE_STATUS == 0) {
    int server_socket, socket_desc, client_size;
    int no_of_conn = 0;
    int client_sockets[NUMBER_OF_CLI];
    struct sockaddr_in client_addresses[NUMBER_OF_CLI];
    struct sockaddr_in server_address;
    socklen_t client_address_len;




    // SOCKET CONFIGURATION
    //
    //
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    // Create server socket
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT_NUMBER);
    server_address.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Bind the socket descriptor to the server address (the port and IP):
    if(bind(socket_desc, (struct sockaddr*)&server_address, sizeof(server_address))<0){
        printf("Couldn't bind to the port\n");
        return -1;
    }
    printf("Socket binding: DONE\n");

    // Turn on the socket to listen for incoming connections:
    if(listen(socket_desc, 1) < 0){
        printf("Error while listening\n");
        return -1;
    }
    printf("\nListening for incoming connections.....\n");


    client_size = sizeof(client_addresses[0]);
    for (int i = 0; i < NUMBER_OF_CLI; i++) {
        client_sockets[i] = 0;
    }

    while (1) {
        // Accept incoming connection
        int new_socket = accept(socket_desc, (struct sockaddr*)&client_addresses[no_of_conn], (socklen_t *)&client_size);
        if (new_socket < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        printf("New client connected\n");

        // Add new client to array
        for (int i = 0; i < NUMBER_OF_CLI; i++) {
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

    //
    //
    // END OF SOCKET CONFIGURATION

    // tasks: generate random NxN
    // split into N/t x N
    // loop for every client and send the matrix/struct with the appropriate port

    // I. Initializing the input matrix

    


    srand(time(NULL));
    char **xVec = (char**) malloc(sizeof(char*) * NumberOfInputs);
    double *yVec = (double*) malloc(sizeof(double) * NumberOfInputs);
    double *rVec = (double*) malloc(sizeof(double) * NumberOfInputs); 
    xVec[0] = malloc(sizeof(char) * NumberOfInputs);
    for(int j = 0; j < NumberOfInputs; j++){
        xVec[0][j] = rand() % 100 + 1;
        yVec[j] = rand() % 20 + 1;
    }
    for(int i = 1; i < NumberOfInputs; i++) {
        xVec[i] = malloc(sizeof(char) * NumberOfInputs);
        memcpy(xVec[i], xVec[0], sizeof(xVec[0][0]) * NumberOfInputs);
    }

    // turn them into arg
    // packetizing
    // matrix_packet* packets = (matrix_packet*) malloc(sizeof(matrix_packet) * NUMBER_OF_CLI);
    // for(int i = 0; i < NUMBER_OF_CLI; i ++) {
    //     packets[i].id = i;
    //     packets[i].no_of_cols = NumberOfInputs/NUMBER_OF_CLI;
    //     if (i == NUMBER_OF_CLI-1) packets[i].no_of_cols += (NumberOfInputs%NUMBER_OF_CLI);

    //     // we will be turning the 2d array into 1d array
    //     // debug: if ever segfault, change the size of the memcpy. since we are 
    //     packets[i].dataCol = malloc(sizeof(char) * packets[i].no_of_cols * NumberOfInputs);
    //     int index = 0;
    //     for(int j = 0; j < packets[i].no_of_cols; j++) {
    //         int min_cols_per_packet = NumberOfInputs/NUMBER_OF_CLI;
    //         memcpy(&packets[i].dataCol[j*NumberOfInputs], xVec[i*min_cols_per_packet + j], NumberOfInputs);
    //     }
    //     packets[i].yDataCol = malloc(sizeof(double) * NumberOfInputs);
    //     memcpy(packets[i].yDataCol, yVec,sizeof(double) * NumberOfInputs); 
    // }







// II. Packetizing, splitting of matrix for each 
    struct timespec before = {0,0}, after = {0,0};
    clock_gettime(CLOCK_MONOTONIC, &before);

    // spawn threads
    thread_args* argHolder = (thread_args*) malloc(sizeof(thread_args) * NUMBER_OF_CLI);
    pthread_t *tid = (pthread_t*) malloc(sizeof(pthread_t) * NUMBER_OF_CLI);
    
    for (int i = 0; i < NUMBER_OF_CLI; i++)  {
        argHolder[i].no_of_cols = NumberOfInputs/NUMBER_OF_CLI;
        if (i == NUMBER_OF_CLI-1) argHolder[i].no_of_cols += (NumberOfInputs%NUMBER_OF_CLI);
        
        argHolder[i].no_of_inputs = NumberOfInputs;
        argHolder[i].no_of_clients = NUMBER_OF_CLI;
        argHolder[i].id = i;
        argHolder[i].result = malloc(sizeof(double) * argHolder[i].no_of_cols);
        argHolder[i].client_socket = client_sockets[i];
        argHolder[i].rVec = rVec;
        argHolder[i].yVec = yVec;
        argHolder[i].xVec = xVec;

        pthread_create(&tid[i], NULL, recieve_client, (void *) &argHolder[i]);
        printf("Created thread #%d\n",i);
    }



    


    // // sending ID, no_of_cols, dataCol, yDataCol
    // // migrate this into threads
    // for (int i = 0; i < NUMBER_OF_CLI; i++)  {
    //     if (send(client_sockets[i], &packets[i].id, sizeof(packets[i].id), 0) < 0){
    //         printf("Can't send\n");
    //         return -1;
    //     }

    //     if (send(client_sockets[i], &packets[i].no_of_cols, sizeof(packets[i].no_of_cols), 0) < 0){
    //         printf("Can't send\n");
    //         return -1;
    //     }

    //     if (send(client_sockets[i], packets[i].dataCol, sizeof(char) * packets[i].no_of_cols * NumberOfInputs, 0) < 0){
    //         printf("Can't send\n");
    //         return -1;
    //     }

    //     if (send(client_sockets[i], packets[i].yDataCol, sizeof(double) * NumberOfInputs, 0) < 0){
    //         printf("Can't send\n");
    //         return -1;
    //     }
    // }

    // display_table(xVec, NumberOfInputs, NumberOfInputs, yVec);       DEBUG

    // joining of threads
    for (int i = 0; i < NUMBER_OF_CLI; i++)  {
        pthread_join(tid[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &after);
    double runtime = (((double) 1000*after.tv_sec + after.tv_nsec/1000000.0) - ((double) 1000*before.tv_sec + before.tv_nsec/1000000.0));
            printf("-- RUNTIME --\n");
            printf("%lf ms\n", runtime);
             printf("-------------\n");



    printf("integrity check: %lf\n",rVec[0]);









    // Client code
    return 0;
    } else if (INSTANCE_STATUS == 1) {

            int NumberOfInputs = atoi(argv[1]);
            int PORT_NUMBER = atoi(argv[2]);
            int INSTANCE_STATUS = atoi(argv[3]);

            int socket_desc;
            struct sockaddr_in server_addr;

            socket_desc = socket(AF_INET, SOCK_STREAM, 0);
            if(socket_desc < 0){
                printf("Unable to create socket\n");
                return -1;
            }

            // Set port and IP the same as server-side:
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(PORT_NUMBER);
            server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

            if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
                printf("Unable to connect\n");
                return -1;
            }
            printf("Connected with server successfully\n");

            int client_id = 0;
            int client_cols = 0;

            // Receive the server's response:
            // At this point we should recieve 3 things
            if(recv(socket_desc, &client_id, sizeof(client_id), 0) < 0){
                printf("Error while receiving server's msg\n");
                return -1;
            }

            if(recv(socket_desc, &client_cols, sizeof(client_cols), 0) < 0){
                printf("Error while receiving server's msg\n");
                return -1;
            }

            

            char* client_array_buffer = malloc(sizeof(char) * NumberOfInputs);
            char** twod_client_array = malloc(sizeof(char*)*client_cols);
            int sendme = 1;
            int total_recieved = 0;
            for(int i = 0 ; i < client_cols ; i++) {
            	// printf("Recieved X: [%d]\n", i);
                twod_client_array[i] = malloc(sizeof(char) * NumberOfInputs);
                
                
                // while loop to recieve all the message
                while(total_recieved < sizeof(char)*NumberOfInputs) {
		        int recvstat = recv(socket_desc, twod_client_array[i], sizeof(char)*NumberOfInputs,0);
		        if (recvstat <= 0) {
				printf("error\n");
				return -1;
		        }
		        total_recieved += recvstat;
                }
                total_recieved = 0;
            	
            	// send an ack
            	// printf("Sending ack for xVec %d\n",i);
            	send(socket_desc, &sendme, sizeof(int), 0);
            }

            double* client_y_array = malloc(sizeof(double) * NumberOfInputs);
            double* return_array = malloc(sizeof(double) * client_cols);

	   int total_recieved = 0;
	   
	   while(total_recieved < sizeof(double)*NumberOfInputs) {
	   	int revstat = recv(socket_desc, client_y_array, sizeof(double) * NumberOfInputs, 0);
            if(revstat < 0){
                printf("Error while receiving server's msg\n");
                return -1;
            }
            total_recieved += revstat;
	   }
	   

            // ok now we have the 1d array, lets convert it to 2d
            


            // Filling in the arguments for the threads
                int ofst = client_cols / NUMBER_OF_THREADS;
                arg* argHolder = (arg*) malloc(sizeof(arg) * NUMBER_OF_THREADS);
                pthread_t *tid = (pthread_t*) malloc(sizeof(pthread_t) * NUMBER_OF_THREADS);

                for(int i = 0; i < NUMBER_OF_THREADS; i++) {
                    int start = ofst*i;
                    int end = ofst*(i+1);

                    if (i == NUMBER_OF_THREADS-1) end = client_cols;

                    printf("s: %d, e: %d\n",start, end);

                    argHolder[i].x = twod_client_array;
                    argHolder[i].y = client_y_array;
                    argHolder[i].n = NumberOfInputs;
                    argHolder[i].c  = client_cols;
                    argHolder[i].r = return_array;
                    argHolder[i].start = start;
                    argHolder[i].end = end;
                }
            
            // display_table(twod_client_array, NumberOfInputs, client_cols, client_y_array);

            pearson_cor(twod_client_array, client_y_array, NumberOfInputs, return_array, NUMBER_OF_THREADS, argHolder, tid);
            printf("integrity check: %lf\n", return_array[0]);
            
            if(send(socket_desc, return_array, sizeof(double) * client_cols, 0) < 0){
                printf("Unable to send message\n");
                return -1;
            } 

        
    }
}
