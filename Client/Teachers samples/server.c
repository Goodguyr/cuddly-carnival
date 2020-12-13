#include "game_lib.h"

#include<errno.h>
#include<sys/wait.h>
#include<sys/mman.h>

char* shared_memory = NULL;
/* SHARED MEMORY use as follows
   char = player_count
   char = game_id
   int = seconds left to play
   int = dot count
   MAX_PLAYERS * SHARED_MEMORY_PER_CLIENT = incoming packets per player
      For each player:
        1 byte = {0: processed, waiting new packet, 1: packet received and stored, 2: packet is being red by gameloop - don't touch }
        2..N bytes the packet itself without separators and checksums (already un-escaped)
   maximum_packet_size = Outgoing game state packet
   SHARED_MEMORY_PER_CLIENT = outgoing message buffer
   MAX_PLAYERS*(285+3*sizeof(float))
      1 = status
      1 = current_keypresses
      4 = player size as int
      1 = nick length
      256 = nickname
      3*sizeof(float) = player color RGB
      6 = player color in HEX chars
      4 = number of lives
      4 = x coordinate
      4 = y coordinate
      4 = player score
   MAX_DOTS * 8
      4 = x coordinate
      4 = y coordinate
*/

char* outgoing_packet = NULL;
char* outgoing_message_buffer = NULL;
char* game_state = NULL;
char* dot_state = NULL;

unsigned char* client_count = NULL;
unsigned char* game_id = NULL;
unsigned int* seconds_left = NULL;
unsigned int* dot_count = NULL;

unsigned int max_x = 1000;
unsigned int max_y = 1000;
unsigned int player_initial_size = 5;
unsigned int initial_timer_value = 120;
unsigned int default_lives = 3;

int player_data_size = 285 + 3*sizeof(float);
int player_color_offset = 2+4+1+256;
int player_lives_offset = 0;
int player_coordinate_offset = 0;
int player_score_offset = 0;

int outgoing_packet_id = 0;


void get_shared_memory();
void gameloop();
void start_network(char* port);
void process_client(unsigned char id, int socket);
void process_packets();

int main(int argc, char** argv){
  char* port;
  if(argc==2){
    port = argv[1];
  } else {
    printf("Please provide PORT in terminal (e.g. server 12345)\n");
    exit(0);    
  }

  init_hardware();
  player_lives_offset = player_color_offset + 3*sizeof(float) + 6;
  player_coordinate_offset = player_lives_offset + 4;
  player_score_offset = player_coordinate_offset + 8;


  int pid = 0;
  int i;
  printf("SERVER started!\n");
  get_shared_memory();
  /* fork to have two processes -
     networking (child here) and 
     game loop (parent here) */
  pid = fork();
  if(pid == 0){
    start_network(port);
  } else {
    gameloop();
  }
  return 0;
}

void get_shared_memory(int size){
  int total_shared_memory_size = 2 + 2* sizeof(int) +
      MAX_PLAYERS * SHARED_MEMORY_PER_CLIENT +
      maximum_packet_size + SHARED_MEMORY_PER_CLIENT +
      MAX_PLAYERS * player_data_size +
      sizeof(int) + MAX_DOTS * 8;
  shared_memory = mmap(NULL, total_shared_memory_size, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
  client_count = (unsigned char*) shared_memory;
  game_id = (unsigned char*)(shared_memory+1);
  seconds_left = (unsigned int*)(shared_memory+2);
  dot_count = (unsigned int*)(seconds_left+sizeof(int));
  outgoing_packet = shared_memory + 2 + 2*sizeof(int) + MAX_PLAYERS * SHARED_MEMORY_PER_CLIENT;
  outgoing_message_buffer = outgoing_packet+maximum_packet_size;
  game_state = outgoing_message_buffer + SHARED_MEMORY_PER_CLIENT;
  dot_state = game_state + MAX_PLAYERS * player_data_size;

}

char* get_client_packet_memory(unsigned char npk){
  return shared_memory + 2 + 2*sizeof(int) + npk * SHARED_MEMORY_PER_CLIENT;
}

char* get_client_data_memory(unsigned char npk){
  return game_state + npk * player_data_size;

}

void gameloop(){
  printf("Starting game loop! (It will run forever - use Ctrl+C)\n");
  char i=0;
  *game_id = 0;
  while(*game_id<255){
    printf("Created game ID=%d, waiting for players!", *game_id);
    while(1){
      /*wait for game beginning, ready */
      process_packets();
      sleep(1);
    }
    while(1){
      printf("Game ID=%d starting!\n", *game_id);
      process_packets();

      for(i=0; i<*client_count;i++){
        /* print_bytes(get_client_packet_memory(i), 100);*/ 
      } 
      sleep(1);
    }
    printf("Ended game ID=%d", *game_id);
    *game_id++;
  }
}


void process_client_request_packet(unsigned char player_id, char* packet){
  char* data = packet+8;
  unsigned char i = 0;
  unsigned char nick_len = data[0];
  char* player_data = get_client_data_memory(player_id);
  player_data[6] = nick_len;
  for(i=0;i<nick_len;i++){
    player_data[7+i] = data[1+i];
  }
  player_data[7+nick_len] = 0;
  printf("Received request from NICK='%s'\n", player_data+7);
  /*player_data[player_color_offset] TODO colors */
  player_data[0] = 1; /* set up state, so that approval can be sent*/
}


void process_packets(){
  unsigned char i = 0;
  char* pack = 0;
  unsigned char typ = 0;
  for(i=0; i<*client_count; i++){
    pack = get_client_packet_memory(i);
    if(pack[0]==1){
      pack[0]=2;
      /* process packet*/
      typ = pack[1];
      if(typ == 0){
        process_client_request_packet(i, pack+2);
      } else {
        printf("TODO: Packet type not implemented yet!\n");
      }
      pack[0]=0;
    }
  }
  
}



void start_network(char* port){
  int main_socket = start_server_connection(port);
  int client_socket;
  struct sockaddr_in client_address;
  int client_address_size = sizeof(client_address);
  *client_count = 0;

  while(1){
    unsigned char new_client_id = 0;
    int cpid = 0;
    /* waiting for clients */
    client_socket = accept(main_socket, (struct sockaddr*) &client_address, &client_address_size);
    if(client_socket<0)
    { printf("Error accepting client connection! ERRNO=%d\n", errno); continue; } 
    /* if client connection fails, we can still accept other connections */
    if(*client_count==255) {
      printf("Error: exceeded maximum number of clients!\n");
      close(client_socket);
      continue;
    }
    new_client_id = *client_count;
    *client_count +=1;
    cpid = fork();

    if(cpid == 0){
      /* start child connection processing */
      close(main_socket);
      cpid = fork();
      if(cpid == 0){
        process_client(new_client_id, client_socket);
        exit(0);
      } else {
        /* orphaning */
        wait(NULL);
        printf("Successfully orphaned clinet %d\n", new_client_id);
        exit(0);
      }
    } else {
      close(client_socket);
    }
  }
} /* finish main() */


void send_approval_packet(int socket, unsigned char game_id, unsigned char player_id, unsigned int initial_size, unsigned int x, unsigned int y, unsigned int time, unsigned int lives){
  unsigned int size;
  char packet[100] = {1};
  char raw_packet[100];
  put_4_bit_integer(outgoing_packet_id, packet+5);
  outgoing_packet_id++;

  packet[9] = game_id;
  packet[10] = player_id;

  put_4_bit_integer(initial_size, packet+11);
  put_4_bit_integer(x, packet+15);
  put_4_bit_integer(y, packet+19);
  put_4_bit_integer(time, packet+23);
  put_4_bit_integer(lives, packet+27);
  
  size = 22;
  put_4_bit_integer(size, packet+1);

  packet[31] = calculate_checksum(packet, 31);

  size = prepare_and_escape_packet(packet, raw_packet, 32);
  send(socket, raw_packet, size, 0);
  print_bytes(packet,32);
  print_bytes(raw_packet,size);
}


void send_packets(int socket, char* data, unsigned char id){
  unsigned char state =0;
  while(1){
    state = data[0];
    if(state==1){
      send_approval_packet(socket, *game_id, id, get_4_bit_integer(data+2), get_4_bit_integer(data+player_coordinate_offset), get_4_bit_integer(data+4+player_coordinate_offset), get_4_bit_integer(data+2), get_4_bit_integer(data+player_lives_offset));
    }
  }
}

void process_client(unsigned char id, int socket){
  char* mem = get_client_data_memory(id);
  int i = 0;
  unsigned int* tmp;
  printf("Processing client id=%d, socket=%d\n", id, socket);
  printf("CLIENT count %d\n",*client_count);
  /* Setup client information */
  for(i=0;i<player_data_size;i++) mem[i] = 0;
  tmp = (unsigned int*)(mem+2);
  *tmp = player_initial_size;
  tmp = (unsigned int*)(mem + player_lives_offset);
  *tmp = default_lives;

  int pid=0;

  pid=fork();
  if(pid == 0){
    /* enter infinite reading loop */
    read_packets_to_buffer(socket, get_client_packet_memory(id));
  } else {
    send_packets(socket, get_client_data_memory(id), id);
  }

}
