#ifndef GAME_H
#define GAME_H
#include "../protocol.h"

int* shared_memory;
int* client_count;
int* shared_data;
int* CLIENT_REQUEST;
struct PlayerUpdate* player_data;
struct FoodInfo* foodData;

//<========= Game Data =========>
int GAME_ID;
int INITIAL_PLAYER_SIZE;
int TIME_LIMIT;
int LIVE_LIMIT;
int PLAYFIELD_X;//5000
int PLAYFIELD_Y;//5000

int MY_HIGHSCORE;
int GAME_RUNNING;

int PACKET_COUNTER;

unsigned char buffer[BUFF_SIZE];
unsigned char readBuffer[BUFF_SIZE];
unsigned char* largeBuffer;

void gameloop();
void game_init();
void client_connection(int id, int client_fd);
void setup_game_params();
void fillFoodData();
void server_client_write(int client_id, int client_fd);
void server_client_read(int client_id, int client_fd);

#endif