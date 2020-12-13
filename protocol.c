#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include <ctype.h>
#include <inttypes.h>

#include "protocol.h"

char printable_char(char c){
  if(isprint(c)!=0) return c;
  return ' ';
}

void print_bytes(void* packet, int count){
  int i;
  char *p = (char*) packet;
  printf("Printing %d bytes...\n", count);
  printf("[NPK] [C] [HEX] [DEC] [BINARY]\n");
  printf("==============================\n");
  for(i=0; i<count; i++){
    printf(" %3d | %c |  %02X | %3d | %c%c%c%c%c%c%c%c\n",i,printable_char(p[i]), (unsigned char) p[i], (unsigned char) p[i],
      p[i] & 0x80 ? '1' : '0',
      p[i] & 0x40 ? '1' : '0',
      p[i] & 0x20 ? '1' : '0',
      p[i] & 0x10 ? '1' : '0',
      p[i] & 0x08 ? '1' : '0',
      p[i] & 0x04 ? '1' : '0',
      p[i] & 0x02 ? '1' : '0',
      p[i] & 0x01 ? '1' : '0');

  }
}

int little_endian(){
    int temp = 1;
    unsigned char* testing = (unsigned char*)&temp;
    if(testing[0] == 0){
        return 0;
    }
    return 1;
}

void clear_buffer(unsigned char* buf){
    bzero(buf, 1024);
}

int xor_check(unsigned char* buffer, int len){
    int result = 0;
    int i;
    // len + 11 because 2 bytes are '\0' and 9 is for the header
    for(i = 2; i < len + 11; i++){
        result ^= buffer[i];
    }
    // printf("Cheksum calculated is %d\n", result);
    return result;
}

int getDataSize(unsigned char* buffer){
    int* data_size = (int*)(buffer + 3);
    // printf("Data size is %d\n", *data_size);
    // printf("After ntohl data size is %d\n", ntohl(*data_size));
    return *data_size;
}

int getPacketType(unsigned char* buffer){
    return (int)buffer[2];
}


void encode(unsigned char* buffer, int size){
    int i;
    // len + 1 because 2 bytes are '\0' and 9 is for the header and 1 for checksum
    for(i = 2; i < size + 12; i++){
        if(buffer[i] == '\0'){
            buffer[i] = (unsigned char) 12;
        }
        else if(buffer[i] == (unsigned char) 1){
            buffer[i] = (unsigned char) 13;
        }
    }
}

void decode(unsigned char* buffer){
    int i = 2;
    while(buffer[i] != '\0'){
        if(buffer[i] == (unsigned char) 12){
            buffer[i] = '\0';
        }
        else if(buffer[i] == (unsigned char) 13){
            buffer[i] = (unsigned char) 1;
        }
        i++;
    }
}

void addByteToData(unsigned char* buffer, unsigned char n, int* data_size){
    unsigned char temp = n;
    memcpy(buffer + (int)(*data_size), &temp, sizeof(n));
    *data_size += sizeof(n);
}

void addIntToData(unsigned char* buffer, int n, int* data_size){
    int num = n;
    unsigned char* temp = (unsigned char*)&num;
    memcpy(buffer + *data_size, temp, sizeof(int));
    *data_size += sizeof(n);
}

void addStringToData(unsigned char* buffer, unsigned char* word, int* data_size){
    memcpy(buffer + *data_size, word, strlen((char*)word));
    *data_size += strlen((char*)word);
}

void addShortIntToData(unsigned char* buffer, short int n, int* data_size){
    int num = n;
    unsigned char* temp = (unsigned char*)&num;
    memcpy(buffer + *data_size, temp, sizeof(short int));
    *data_size += sizeof(n);
}

void sendPacket(unsigned char* buffer, int len, int server_fd){
    //+14 because 9 is size of header and 4 is size of dividers, 1 for checksum
    send(server_fd, buffer, len + 14, 0);
}

void addDataToPacket(unsigned char* buffer, unsigned char* word, int string_size, int* data_size){
    memcpy(buffer + *data_size, word, string_size);
    *data_size += string_size;
}

void makePacket(unsigned char* buffer, pType_e event, int data_size, int packet_count, unsigned char* data){
    clear_buffer(buffer);
    int length = 2;
    addByteToData(buffer, event, &length);
    addIntToData(buffer, data_size, &length);
    addIntToData(buffer, packet_count, &length);
    addDataToPacket(buffer, data, data_size, &length);
    // addStringToData(buffer, data, &length);
    unsigned char checksum = xor_check(buffer, data_size);
    addByteToData(buffer, checksum, &length);
    // memcpy(buffer + length, &checksum, sizeof(unsigned char));
    encode(buffer, data_size);
}