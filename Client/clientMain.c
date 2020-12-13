#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "client.h"


int main(int argc, char** argv){
    // allowedToPlay = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    // setupDone = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    // *allowedToPlay = 0;
    // *setupDone = 0;

    foodInfo = mmap(NULL, sizeof(struct Food) * FOOD_AMOUNT, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    playerData = mmap(NULL, sizeof(struct Player) * MAX_CLIENTS, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    myPlayer = mmap(NULL, sizeof(struct Player*), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    myPlayer[0] = mmap(NULL, sizeof(struct Player), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    char* ip_address = malloc(128);
    int port_no = 0;
    char buffer[BUFF_SIZE];
    if(get_argument(argc, argv, 'p', buffer)){
        port_no = atoi(buffer);
    }
    if(get_argument(argc, argv, 'a', buffer)){
        strcpy(ip_address, buffer);
    }
    if(!port_no || !ip_address ){
        printf("Incorrect argument list.\n");
        printf("Example: %s -p=<port> -a=<ip address>\n", argv[0]);
        exit(0);
    }
    start_client(ip_address, port_no);
    return 0;
}

void initialize_client_connection(int argc, char** argv){
    foodInfo = mmap(NULL, sizeof(struct Food) * FOOD_AMOUNT, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    playerData = mmap(NULL, sizeof(struct Player) * MAX_CLIENTS, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    myPlayer = mmap(NULL, sizeof(struct Player*), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    myPlayer[0] = mmap(NULL, sizeof(struct Player), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    char* ip_address = malloc(128);
    int port_no = 0;
    char buffer[BUFF_SIZE];
    if(get_argument(argc, argv, 'p', buffer)){
        port_no = atoi(buffer);
    }
    if(get_argument(argc, argv, 'a', buffer)){
        strcpy(ip_address, buffer);
    }
    if(!port_no || !ip_address ){
        printf("Incorrect argument list.\n");
        printf("Example: %s -p=<port> -a=<ip address>\n", argv[0]);
        exit(0);
    }
    start_client(ip_address, port_no);
}
