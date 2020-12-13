#ifndef CLIENT_H
#define CLIENT_H
#include "../protocol.h"

unsigned char readBuffer[1024 * 30];
unsigned char buffer[1024];
struct PlayerUpdate* playerData;
struct FoodInfo* foodInfo;

struct Player** myPlayer;

static int game_id;
static int allowedToPlay = 0;
static int setupDone = 0;
static int request_sent = 0;

int my_id;


void player_init();
void* server_connection_send(void* server_fd_ptr);
void server_connection_read(int client_socket);
void start_client(char* ip_address, int port_no);
int get_argument(int argc, char** argv, char type, char* buffer);
void initialize_client_connection(int argc, char** argv);
int playerInScreen(struct Player* myPlayer, struct Player* otherPlayer);
double getRadius(struct Player* player);
float set_zoom_value(double size);

#endif