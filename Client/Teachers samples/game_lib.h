#ifndef MY_GAME_HEADER
#define MY_GAME_HEADER

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<sys/socket.h>

#define MAX_PLAYERS 30
#define MAX_DOTS 50
#define SHARED_MEMORY_PER_CLIENT 526
#define MINIMUM_PACKET_SIZE 10

extern char is_little_endian;
extern int maximum_packet_size; /*witohut dividers */

void init_hardware();

char printable_char(char c);

int get_4_bit_integer(void * addr);

void put_4_bit_integer(unsigned int data, void* addr);

int unescape(char* ch);

int prepare_and_escape_packet(char* buffer_in, char* buffer_out, int n);

char calculate_checksum(char* buffer, int n);

void print_bytes(void* packet, int count);

char check_if_little_endian_system();

int packet_is_ok(char* buffer, int n, int last_id);

int start_client_connection(char* host, char* port);

int start_server_connection(char* port);

int read_packets_to_buffer(int socket, void* target_buffer);


#endif