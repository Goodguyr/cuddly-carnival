#ifndef PROTOCOL_H
#define PROTOCOL_H

#define BUFF_SIZE 1024
#define MAX_CLIENTS 20
#define USERNAME_LENGTH 256
#define FOOD_AMOUNT 1000

typedef enum pType_e{
    Client_request,
    Approval,
    Client_ready,//<---- Is not required
    Game_state,
    Player_update,
    Game_lost,
    Game_ended,
    Message,
} pType_e;


struct Position{
    int x;
    int y;
};

struct Player{
    int id;
    int size;
    unsigned char username[USERNAME_LENGTH];
    unsigned char color[8];
    struct Position position;
    int lives;
    int status;
};

struct Food{
    struct Position position;
};

struct FoodInfo{
    struct Food foodPosition[FOOD_AMOUNT];
};

struct PlayerUpdate{
    struct Player allPlayers[MAX_CLIENTS];
};

struct KeyboardInput{
    int x;
    int y;
};

typedef struct __attribute__((__packed__)) GameSetup{
    char game_id;
    char player_id;
    int player_size;
    int map_size[2];
    unsigned int time_limit;
    unsigned int lives_count;
}GameSetup;

typedef struct __attribute__((__packed__)) ReceivedData{
    unsigned char type;
    int data_size;
    int packet_count;
    char data[BUFF_SIZE];
} ReceivedData;


int little_endian();
void clear_buffer(unsigned char* buf);
int xor_check(unsigned char* buffer, int len);
int getDataSize(unsigned char* buffer);
void encode(unsigned char* buffer, int size);
void decode(unsigned char* buffer);
void sendPacket(unsigned char* buffer, int len, int server_fd);
void makePacket(unsigned char* buffer, pType_e event, int data_size, int packet_count, unsigned char* data);
void addByteToData(unsigned char* buffer, unsigned char n, int* data_size);
void addIntToData(unsigned char* buffer, int n, int* data_size);
void addStringToData(unsigned char* buffer, unsigned char* word, int* data_size);
void sendPacket(unsigned char* buffer, int len, int server_fd);
int getPacketType(unsigned char* buffer);
void addDataToPacket(unsigned char* buffer, unsigned char* word, int string_size, int* data_size);
void addShortIntToData(unsigned char* buffer, short int n, int* data_size);

void print_bytes(void* packet, int count);
#endif