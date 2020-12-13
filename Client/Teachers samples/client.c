#include "game_lib.h"
#include<signal.h>

/*             DIV T N_DAT   NPK      C  NICK
/*             0,0,0,11,0,0,0,0,0,0,0,4,'n','i','c','k','F','F','0','0','0','0',?,0,0 */
char pack0[100] = {0,0,1,2,11,1,2,1,2,1,2,1,2,1,2,1,2,1,2,4,'n','i','c','k','F','F','0','0','0','0',20,0,0};

char* shared_memory = NULL;

void listen_to_socket(int socket);
void send_player_input(int socket);
void gameloop();

int main(int argc, char** argv){
  char* host;
  char* port;
  int pid = 0;
  if(argc==3){
    host = argv[1];
    port = argv[2];
  } else {
    printf("Please provide IP and PORT in terminal (e.g. client 127.0.0.1 12345)\n");
    exit(0);
  }

  init_hardware();

  int game_socket = 0;
  printf("Starting the client!\n");
  game_socket = start_client_connection(host, port);
  if(game_socket>0) {
    printf("Connected to server!\n");

    pid = fork();
    if(pid == 0){
      listen_to_socket(game_socket);
    } else {
      pid = fork();
      if(pid == 0){
        send_player_input(game_socket);
      } else {
        close(game_socket);
        gameloop();  
      }      
    }

  } else {
    printf("ERROR: Could not connect to server host:port=%s:%s", host, port);
  }
  return 0;
}


void listen_to_socket(int sock){
  char buf[1000];
  read_packets_to_buffer(sock, buf);

}

void send_player_input(int sock){
  send(sock, pack0, 33, 0);
}

void gameloop(){
  while(1){}
}
