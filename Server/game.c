#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <raylib.h>
#include <math.h>
#include "game.h"

double getRadius(struct Player* player){
    return sqrt(player->size / M_PI);
}

void fillFoodData(){
    srand(time(NULL));
    for(int i = 0; i < FOOD_AMOUNT; i++){
        foodData->foodPosition[i].position.x = rand() % PLAYFIELD_X;
        foodData->foodPosition[i].position.y = rand() % PLAYFIELD_Y;
    }
}

void place_player(int id){
    srand(time(NULL));
    player_data->allPlayers[id].position.x = rand() % PLAYFIELD_X;
    player_data->allPlayers[id].position.y = rand() % PLAYFIELD_Y;
}

void gameloop(){
    while(1){
        for(int i = 0; i < *client_count; i++){
            for(int j = 0; j < FOOD_AMOUNT; j++){
                Vector2 player, foodPartice;
                player.x = player_data->allPlayers[i].position.x;
                player.y = player_data->allPlayers[i].position.y;
                foodPartice.x = foodData->foodPosition[j].position.x;
                foodPartice.y = foodData->foodPosition[j].position.y;
                int collisionDetected = CheckCollisionCircles(player, getRadius(&player_data->allPlayers[i]), foodPartice, 1);
                if(collisionDetected){
                    foodData->foodPosition[j].position.x = 0;
                    foodData->foodPosition[j].position.y = 0;
                    player_data->allPlayers[i].size += 3;
                }
            }
            for(int j = i + 1; j < *client_count; j++){
                Vector2 firstCircle, secondCircle;
                firstCircle.x = player_data->allPlayers[i].position.x;
                firstCircle.y = player_data->allPlayers[i].position.y;

                secondCircle.x = player_data->allPlayers[j].position.x;
                secondCircle.y = player_data->allPlayers[j].position.y;

                int collisionDetected = CheckCollisionCircles(firstCircle, getRadius(&player_data->allPlayers[i]), secondCircle, getRadius(&player_data->allPlayers[j]));

                if(collisionDetected){
                    struct Player* loosingPlayer = 0;
                    if(player_data->allPlayers[i].size > player_data->allPlayers[j].size * 1.1){
                        // player_data->allPlayers[i].size += player_data->allPlayers[j].size;
                        // player_data->allPlayers[j].lives--;
                        loosingPlayer = &player_data->allPlayers[j];
                    }
                    if(player_data->allPlayers[j].size > player_data->allPlayers[i].size * 1.1){
                        loosingPlayer = &player_data->allPlayers[i];
                    }
                    if(loosingPlayer){
                        loosingPlayer->lives--;
                        if(loosingPlayer->lives){
                            loosingPlayer->size = INITIAL_PLAYER_SIZE;
                        }
                        else{
                            loosingPlayer->size = 0;
                        }
                    }
                }

            }
        }
    }
}

void send_approval(int id, int client_fd){
    setup_game_params();
    place_player(id);
    unsigned char data[23];
    int data_size = 0;
    addByteToData(data, (unsigned char)GAME_ID, &data_size);
    addByteToData(data, (unsigned char)id, &data_size);
    addIntToData(data, INITIAL_PLAYER_SIZE, &data_size);
    addIntToData(data, PLAYFIELD_X, &data_size);
    addIntToData(data, PLAYFIELD_Y, &data_size);
    addIntToData(data, TIME_LIMIT, &data_size);
    addIntToData(data, LIVE_LIMIT, &data_size);
    makePacket(buffer, Approval, data_size, 0, data);
    printf("Sending approoval to client...\n");
    // print_bytes(buffer, data_size + 12);
    // printf("\n");
    sendPacket(buffer, data_size, client_fd);
}

void send_game_lost(int id, int client_fd, int time_elapsed){
    unsigned char data[11];
    int data_size = 0;
    addByteToData(data, (unsigned char)GAME_ID, &data_size);
    addByteToData(data, (unsigned char)id, &data_size);
    addIntToData(data, MY_HIGHSCORE, &data_size);
    addIntToData(data, time_elapsed, &data_size);
    makePacket(buffer, Game_lost, 10, 0, data);
    sendPacket(buffer, 10, client_fd);
}

void send_game_state(int id, int client_fd, int time_left){
    unsigned char* data = malloc(*client_count * sizeof(struct Player) + FOOD_AMOUNT * sizeof(struct Food));
    memset(data, 0, *client_count * sizeof(struct Player) + FOOD_AMOUNT * sizeof(struct Food));
    int data_size = 0;
    addByteToData(data, GAME_ID, &data_size);
    // Unclear if a byte or int should be added, as 2 is closer to 1 char is added
    addShortIntToData(data, *client_count, &data_size);
    int i;
    for(i = 0; i < *client_count; i++){
        addByteToData(data, player_data->allPlayers[i].id, &data_size);
        addByteToData(data, (unsigned char)strlen((char*)&player_data->allPlayers[i].username), &data_size);
        addStringToData(data, player_data->allPlayers[i].username, &data_size);
        addIntToData(data, player_data->allPlayers[i].position.x, &data_size);
        addIntToData(data, player_data->allPlayers[i].position.y, &data_size);
        addStringToData(data, player_data->allPlayers[i].color, &data_size);
        addIntToData(data, player_data->allPlayers[i].size, &data_size);
        /* Score is the same as size */
        addIntToData(data, player_data->allPlayers[i].size, &data_size);
        addIntToData(data, player_data->allPlayers[i].lives, &data_size);
    }
    addShortIntToData(data, FOOD_AMOUNT, &data_size);
    for(i = 0; i < FOOD_AMOUNT; i++){
        addIntToData(data, foodData->foodPosition[i].position.x, &data_size);
        addIntToData(data, foodData->foodPosition[i].position.y, &data_size);
    }
    addIntToData(data, time_left, &data_size);
    makePacket(largeBuffer, Game_state, data_size, PACKET_COUNTER, data);
    sendPacket(largeBuffer, data_size, client_fd);
    free(data);
}

void process_keyboard_input(unsigned char* buffer){
    int players_id = (int)(buffer + 12);
    // *players_id = ntohl(*players_id);
    if(*(buffer + 16) == '1'){
        // W pressed
        player_data->allPlayers[players_id].position.y -=  5 - (0.1 * player_data->allPlayers[players_id].size);
    }
    if(*(buffer + 17) == '1'){
        // A pressed
        player_data->allPlayers[players_id].position.x -=  5 - (0.1 * player_data->allPlayers[players_id].size);
    }
    if(*(buffer + 18) == '1'){
        // S pressed
        player_data->allPlayers[players_id].position.y += 5 - (0.1 * player_data->allPlayers[players_id].size);
    }
    if(*(buffer + 19) == '1'){
        // D pressed
        player_data->allPlayers[players_id].position.x += 5 - (0.1 * player_data->allPlayers[players_id].size);
    }
    if(*(buffer + 20) == '1'){
        // ESC pressed
        // Client disconnects on its own
    }
}

void process_client_request(int id, int data_segment_size){
    player_data->allPlayers[id].id = id;
    memcpy(player_data->allPlayers[id].color, &readBuffer[data_segment_size + 5], 6);
    player_data->allPlayers[id].color[6] = '\0';
    memcpy(player_data->allPlayers[id].username, &(readBuffer[12]), (int)(readBuffer[11]));
    player_data->allPlayers[id].username[(int)(readBuffer[11])] = '\0';
    player_data->allPlayers[id].size = INITIAL_PLAYER_SIZE;
    player_data->allPlayers[id].lives = LIVE_LIMIT;
    player_data->allPlayers[id].status = 1;
}

void server_client_read(int id, int client_fd){
    while(1){
        unsigned char get[1];
        clear_buffer(readBuffer);
        int nullCounter = 0;
        int charCounter = 0;
        while(nullCounter != 4){
            read(client_fd, get, 1);
            if(get[0] == '\0'){
                nullCounter++;
            }
            readBuffer[charCounter] = get[0];
            charCounter++;
        }
        if(charCounter == 4){continue;}
        decode(readBuffer);
        // print_bytes(readBuffer, charCounter);
        int data_segment_size = getDataSize(readBuffer);
        int packetType = getPacketType(readBuffer);
        if(packetType == 1){
            packetType = 0;
        }
        printf("Got packet type %d\n", packetType);
        unsigned char checksum_char = readBuffer[data_segment_size + 11];
        int received_checksum = (int)checksum_char;
        int calculated_checksum = xor_check(readBuffer, data_segment_size);
        if(received_checksum != calculated_checksum || (data_segment_size + 14 != charCounter && data_segment_size != 0)){
            printf("Received bad packet!\n");
            if(received_checksum != calculated_checksum){
                printf("Received checksum %d but calculated %d\n", received_checksum, calculated_checksum);
            }
            if(data_segment_size + 14 != charCounter){
                printf("Read %d bytes but should be %d\n", charCounter, data_segment_size + 14);
            }
            // print_bytes(readBuffer, charCounter);
            continue;
        }
        print_bytes(readBuffer, charCounter);
        if(packetType == Client_request){
            // printf("Got client request!\n");
            // print_bytes(readBuffer, charCounter);
            process_client_request(id, data_segment_size);
        }
        else if(packetType == Player_update){
            // printf("Printing player update...\n");
            // print_bytes(readBuffer, charCounter);
            process_keyboard_input(readBuffer);
        }
        else if(packetType == Message){
            // Message packet
        }
    }
}

void custom_sleep(long msec){
#if __APPLE__
    usleep(msec);
#elif __linux__
    struct timespec sleepStruct;
    sleepStruct.tv_sec = msec / 1000000;
    sleepStruct.tv_nsec = (msec % 1000000) * 1000000;
    nanosleep(&sleepStruct, &sleepStruct);
#endif
}


// This can be gameloop
void server_client_write(int id, int client_fd){
    // Start ----- should happen on first player join
    while(player_data->allPlayers[id].status != 1){}
    clock_t start = clock();
    printf("Sending approoval\n");
    send_approval(id, client_fd);
    clock_t loop_start = clock();
    while(1){
        clock_t time_elapsed = clock() - start;
        time_elapsed /= CLOCKS_PER_SEC;
        clock_t loop_end = clock();
        /* sleeps if time elapsed is more than 1 / 30 of second due to 30 fps */
        if((float)((loop_end - loop_start) / CLOCKS_PER_SEC) < (1 / 30)){
            long sleep_time = ((loop_end - loop_start) / CLOCKS_PER_SEC) * 1000000;
            printf("sleeping for %ld \n", sleep_time);
            custom_sleep(sleep_time);
        }
        loop_start = clock();
        if(!GAME_RUNNING){
            // Not clear what happens in the end
            // Should send server stopping message
            // int winner_id = get_winner();
            continue;
        }
        // Game connection here
        if(player_data->allPlayers[id].size == 0){
            send_game_lost(id, client_fd, time_elapsed);
        }
        if(player_data->allPlayers[id].size){
            printf("sending game state\n");
            int time_left = TIME_LIMIT - time_elapsed;
            send_game_state(id, client_fd, time_left);
        }
    }
}

void setup_game_params(){
    GAME_ID = 1;
    INITIAL_PLAYER_SIZE = 100;
    LIVE_LIMIT = 1;
    TIME_LIMIT = 300;
    PLAYFIELD_X = 2000;
    PLAYFIELD_Y = 2000;
}

void client_connection(int id, int client_fd){
    // Shared data holds info about chars received
    player_data->allPlayers[id].id = id;
    while(!LIVE_LIMIT){/*waiting for server setup*/}
    *CLIENT_REQUEST = 0;
    int pid = fork();
    if(!pid){
        server_client_read(id, client_fd);
    }
    else{
        server_client_write(id, client_fd);
    }
}