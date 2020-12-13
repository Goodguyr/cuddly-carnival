#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include "../protocol.h"
#include "game.h"


void create_connection(int server_fd){
    printf("Server started.\n");
    setup_game_params();
    fillFoodData();
    int client_fd;
    struct sockaddr_in client_addr;
    while(1){
        int client_process;
        socklen_t client_size = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_size);
        if(client_fd < 0){
            printf("Could not accept a connection\n");
            continue;
        }
        printf("Accepted connection\n");
        int client_id = *client_count;
        *client_count += 1;
        client_process = fork();
        if(!client_process){
            close(server_fd);
            client_process = fork();
            if(!client_process){
                printf("Making connection with id %d\n", client_id);
                client_connection(client_id, client_fd);
                exit(0);
            }
            else{
                wait(NULL);
                printf("Succesfully orphaned a client listener process\n");
                exit(0);
            }
        }
        else{
            close(client_fd);
        }
    }
}

// Setting up socket for listening
void start_server(int port_no){
    struct sockaddr_in server_addr;
    int server_fd;
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("Could not create a socket.\n");
        exit(0);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_no);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    if(bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("Could not bind a socket.\n");
        exit(0);
    }
    if(listen(server_fd, MAX_CLIENTS) < 0){
        printf("Could not listen to this socket.\n");
        exit(0);
    }
    create_connection(server_fd);
}

int get_argument(int argc, char** argv, char type, char* buffer){
    int i;
    char named_argument[] = {'-', type, '=', '\0'};
    for(i = 1; i < argc; i++){
        if(strstr(argv[i], named_argument)){
            strncpy(buffer, argv[i] + 3, strlen(argv[i]) - 2);
            return 1;
        }
    }
    return 0;
}






int main(int argc, char** argv){
    // When connection is made fork it
    // Read data from the client byte by byte

    // After receiving a char send back N times the same char
    // N <-- number of bytes received from the client
    // shared_memory = mmap(NULL, BUFF_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    // player_data = malloc(sizeof(struct Player) * MAX_CLIENTS);
    player_data = mmap(NULL, sizeof(struct PlayerUpdate), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    foodData = mmap(NULL, sizeof(struct FoodInfo), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    client_count = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    if(foodData == MAP_FAILED || client_count == MAP_FAILED || player_data == MAP_FAILED){
        printf("Could not allocate shared memory!\n");
        exit(0);
    }
    *client_count = 0;
    largeBuffer = (unsigned char*)malloc((MAX_CLIENTS) * sizeof(struct Player) + FOOD_AMOUNT * sizeof(struct Food));
    //shared_data = (int*) (shared_memory + sizeof(int));
    CLIENT_REQUEST = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    int port_no;
    char buffer[BUFF_SIZE];
    if(argc != 2){
        printf("Provide the port number for the server.\n");
        printf("Example usage: %s -p=<port number>\n", argv[0]);
        exit(0);
    }
    if(!get_argument(argc, argv, 'p', buffer)){
        printf("Provide the port number for the server.\n");
        printf("Example usage: %s -p=<port number>\n", argv[0]);
        exit(0);
    }
    port_no = atoi(buffer);
    int server_process = fork();
    if(!server_process){
        printf("Starting server...\n");
        start_server(port_no);
    }
    else{
        gameloop();
    }
    return 0;
}
