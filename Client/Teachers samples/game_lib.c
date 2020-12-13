#include "game_lib.h"
#include<stdio.h>
#include<ctype.h>
#include<inttypes.h>

char is_little_endian = 1;
int maximum_packet_size = 0;

char check_if_little_endian_system(){
  /* returns 1 if little and 0 if big endian */
  volatile uint32_t i=0x01234567;
  return (*((uint8_t*)(&i))) == 0x67;
}

void init_hardware(){
  is_little_endian = check_if_little_endian_system();
  /* set max packet size to game state size */
  maximum_packet_size = 10+9+MAX_PLAYERS*287+MAX_DOTS*8;
  /* check if max message fits */
  if(maximum_packet_size<SHARED_MEMORY_PER_CLIENT){
    maximum_packet_size = SHARED_MEMORY_PER_CLIENT;
  }
}

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

int unescape(char* ch){
  /* printf("Un-escaping!\n"); */
  if(ch[0] == 2) ch[0] = 0;
  else if(ch[0] == 3) ch[0] = 1;
  else return 1;
  return 0;
}

int prepare_and_escape_packet(char* buffer_in, char* buffer_out, int n){
  int n_out = 2;
  buffer_out[0] = 0;
  buffer_out[1] = 0;
  int i;
  for(i=0;i<n;i++){
    if(buffer_in[i]==0){
      buffer_out[n_out] = 1;
      n_out++;
      buffer_out[n_out] = 2;
    } else {
      if(buffer_in[i]==1){
        buffer_out[n_out] = 1;
        n_out++;
        buffer_out[n_out] = 3;
      } else {
        buffer_out[n_out] = buffer_in[i];
      }
    }
    n_out++;
  }
  buffer_out[n_out++] = 0;
  buffer_out[n_out++] = 0;
  return n_out;
}

char calculate_checksum(char* buffer, int n){
  int i;
  char res;
  for(i = 0; i<n; i++){
    res ^= buffer[i];
  }
  return res;
}


int get_4_bit_integer(void * addr){
  int res=0;
  int i;
  char* src = addr;
  char* re = (char*) &res;
  for(i=0;i<4;i++){
    if(is_little_endian){
      re[i] = src[i];
    } else {
      re[i] = src[3-i];
    }
  }
  return res;
}

void put_4_bit_integer(unsigned int data, void* addr){
  int i;
  char* destination = addr;
  char* dat = (char*) &data;
  for(i=0;i<4;i++){
    if(is_little_endian){
      destination[i] = dat[i];
    } else {
      destination[i] = dat[3-i];
    }
  }
}

int packet_is_ok(char* buffer, int n, int last_id){
  int i;
  int new_id = 0;
  int length = 0;
  char goal_checksum = buffer[n-1];
  char current_checksum = 0;
  /* ID either 0 or bigger than previous? */
  new_id = get_4_bit_integer(buffer+5);
  if(new_id>0 && new_id<=last_id){
    printf("Received packet out of order! Last ID = %d, received %d\n", last_id, new_id);
    return 0;
  }
  /* Data length ok? */
  length = get_4_bit_integer(buffer+1);
  if(n-10 != length){
    printf("Received packet with bad data size! Needed = %d, received %d\n", n-10, length);
    return 0;
  }

  /* checksum ok? */
  current_checksum = calculate_checksum(buffer, n-1);
  if(current_checksum != goal_checksum){
    printf("Packet chekchsum failed! received = %d, calculated %d\n", goal_checksum, current_checksum);
    return 0;
  }
  return 1;
}

int start_client_connection(char* host, char* port){
  int client_file_descriptor;
  struct addrinfo hints, *list_of_addresses, *a;

  printf("Starting client connection to %s:%s\n", host, port);

  memset(&hints, 0, sizeof(struct addrinfo)); /* zero whole hints */
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM; /* protocol independent stream */

  getaddrinfo(host, port, &hints, &list_of_addresses);

  /* Iterate over whole address list and try to connect */
  for(a = list_of_addresses; a!=NULL; a = a->ai_next){
    if((client_file_descriptor = socket(a->ai_family, a->ai_socktype, a->ai_protocol)) <0)
      continue; /* failed on this address, try next one */
    if(connect(client_file_descriptor, a->ai_addr, a->ai_addrlen)!=-1)
      break; /* successfully connected */
    close(client_file_descriptor); /*could not connect to socket, close, try another */
  }

  /* clean up */
  freeaddrinfo(list_of_addresses);
  if(a==NULL) return -1; /* All connections failed */
  return client_file_descriptor; /*last connection succeeded */
}

int start_server_connection(char* port){
  int server_file_descriptor;
  int opt_value =1;
  struct addrinfo hints, *list_of_addresses, *a;

  printf("Starting server on port %s\n", port);

  memset(&hints, 0, sizeof(struct addrinfo)); /* zero whole hints */
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM; /* protocol independent stream */
  hints.ai_flags = AI_PASSIVE; /* for passive sockets out of box */

  getaddrinfo(NULL, port, &hints, &list_of_addresses);  

  /* Iterate over whole address list and try to connect/bind */
  for(a = list_of_addresses; a!=NULL; a = a->ai_next){
    if((server_file_descriptor = socket(a->ai_family, a->ai_socktype, a->ai_protocol)) <0)
      continue; /* failed on this address, try next one */
    
    /* Get rid of "Address already in use" produced by previous bind calls */
    setsockopt(server_file_descriptor, SOL_SOCKET, SO_REUSEADDR, (const void*)&opt_value, sizeof(int));

    if(bind(server_file_descriptor, a->ai_addr, a->ai_addrlen)==0)
      break; /* successfully connected */
    close(server_file_descriptor); /*could not connect to socket, close, try another */
  }

  /* clean up */
  freeaddrinfo(list_of_addresses);
  if(a==NULL) {
    /* All connections failed */
    printf("Error opening main server socket!\n");
    exit(1);
  }
  printf("Main socket created!\n");  

  /* start listening to the socket */
  if(listen(server_file_descriptor, MAX_PLAYERS) <0){
    close(server_file_descriptor);
    printf("Error listening to socket!\n");
    exit(1);
  }
  printf("Main socket is listening!\n");

  return server_file_descriptor; /*last connection succeeded */
}



int read_packets_to_buffer(int socket, void* target_buffer){
  char buffer[300];
  char in[1];
  char* ptr = target_buffer;
  int n = 0;
  int i = 0;
  char prev_was_zero = 0;
  char in_packet = 0;
  char escape_mode = 0;
  int last_packet_id = 0;

  printf("Starting to read packets!");
  while(read(socket, in, 1)){
    
    /* if byte is zero - also handle odd zero count */
    if(in[0] == 0){
      /* packet ends */
      if(n<MINIMUM_PACKET_SIZE) {
        /* Discard silently */
        /* printf("Packet too small = %d!\n", n); */
      } else {
        /* new packet, store old */
        if(packet_is_ok(buffer, n, last_packet_id)){
          while(ptr[0]==2){} /* wait if buffer red by server now */
          ptr[0] = 0; /* mark currently no packet */
          for(i=0;i<n-1;i++){
             ptr[i+1] = buffer[i];
          }
          ptr[n] = 0; /* just in case put 0 after packet */
          ptr[0] = 1; /* mark packet as waiting in buffer */
          last_packet_id = get_4_bit_integer(buffer+5);
          printf("Received packet of %d bytes\n",n);
          print_bytes(ptr+1, n-1);
        }
      }        
      n = 0;
      in_packet = 0;
      if(prev_was_zero){
        in_packet = 1;
      } else {
        prev_was_zero = 1;
      }
    } else {
      if(in_packet){
        prev_was_zero = 0;
        /* un-escape if needed */
        if(in[0] == 1){
          escape_mode = 1;
          continue;
        }
        if(escape_mode){          
          escape_mode = 0;
          if(unescape(in) == 1){
            n = 0;
            in_packet = 0;
            continue;
          }         
        }

        buffer[n] = in[0];
        n++;
      }
    }
  }
  printf("The socket ended/is empty!\n");
}