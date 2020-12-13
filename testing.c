#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include "protocol.h"


// int xor_check(char* buffer, int len){
//     int result = 0;
//     int i;
//     for(i = 2; i < len + 2; i++){
//         result ^= buffer[i];
//     }
//     return result;
// }

// struct packet{
//     unsigned char type;
//     int data_size;
//     int packet_num;
//     unsigned char data[1015];
// };


// int getPacketType(unsigned char* buffer){
//     return (int)buffer[2];
// }
// struct PlayerUpdate playerData;
// struct Player myPlayer;

// int game_id;
// int key_w, key_a, key_s, key_d, key_esc;

// int allowedToPlay = 0;
// int setupDone = 0;

void clear_buffer(unsigned char* buf){
    bzero(buf, sizeof(*buf));
}

// int little_endian(){
//     int temp = 1;
//     unsigned char* testing = (unsigned char*)&temp;
//     if(testing[0] == 0){
//         return 0;
//     }
//     return 1;
// }

// void player_init(){
//     printf("IM here \n");
//     printf("Pick a username: ");
//     scanf("%255s", myPlayer.username);
//     printf("\nYour username is: %s\n", myPlayer.username);
//     printf("Pick a HEX color: #");
//     scanf("%6s", myPlayer.color);
//     printf("\nYou picked #%s color\n", myPlayer.color);
//     setupDone = 1;
// }

int main(){
    printf("1 == %d if little endian!!\n", little_endian());
    // unsigned char summ[10000];
    // clear_buffer(summ);
    // unsigned char byteez[] = {0, 0,(unsigned char)100, 0, 0, 0};
    // printf("num is %d\n", getPacketType(byteez));
    return 0;
}