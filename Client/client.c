#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <raylib.h>
#include <math.h>
#include "client.h"


double getRadius(struct Player* player){
    return sqrt(player->size / M_PI);
}

void player_init(){
    printf("Pick a username: ");
    scanf("%255s", myPlayer[0]->username);
    printf("Your username is: %s\n", myPlayer[0]->username);
    printf("Pick a HEX color: #");
    scanf("%7s", myPlayer[0]->color);
    printf("You picked #%s color\n", myPlayer[0]->color);
    myPlayer[0]->id = -1;
    myPlayer[0]->status = 0;
    setupDone = 1;
}

int playerInScreen(struct Player* myPlayer, struct Player* otherPlayer){
    int viewWidth = 100;
    int viewHeight = 100;
    int otherPlayersRadius = getRadius(otherPlayer);
    if(myPlayer->position.x + viewWidth < otherPlayer->position.x - otherPlayersRadius){
        return 0;
    }
    if(myPlayer->position.x - viewWidth > otherPlayer->position.x + otherPlayersRadius){
        return 0;
    }
    if(myPlayer->position.y - viewHeight > otherPlayer->position.y + otherPlayersRadius){
        return 0;
    }
    if(myPlayer->position.y + viewHeight < otherPlayer->position.y - otherPlayersRadius){
        return 0;
    }
    return 1;
}

void updateKeysPressed(unsigned char* keysPressed){
    keysPressed[0] = keysPressed[1] = keysPressed[2] = keysPressed[3] = keysPressed[4] = '0';
    if(IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)){
        keysPressed[0] = '1';
    }
    if(IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)){
        keysPressed[1] = '1';
    }
    if(IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)){
        keysPressed[2] = '1';
    }
    if(IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)){
        keysPressed[3] = '1';
    }
    if(IsKeyDown(KEY_ESCAPE)){
        keysPressed[4] = '1';
    }
}

float set_zoom_value(double size){
    return 6 - size / 5;
}

void* start_client_view(void* map_info, int id){
    // Initialization
    const int screenWidth = 960;
    const int screenHeight = 500;
    struct Position* map_parameters = (struct Position*)map_info;
    const int mapWidth = map_parameters->x;
    const int mapHeight = map_parameters->y;

    InitWindow(screenWidth, screenHeight, "Agar.io clone");
    // Camera set up
    // target should be adjused to player
    // Vector2 ballPosition = {(float)screenWidth/2, (float)screenHeight/2};
    Vector2 playersLocation = {playerData->allPlayers[id].position.x , playerData->allPlayers[id].position.y};
    // printf("Players location is %d x %d\n", myPlayer[0]->position.x, myPlayer[0]->position.y);
    Camera2D camera = {};
    camera.target = playersLocation;
    camera.offset = (Vector2){screenWidth/2, screenHeight/2};
    camera.zoom = set_zoom_value(getRadius(&playerData->allPlayers[id])); // Zoom should depend on size of player

    SetTargetFPS(60);

    // unsigned char keysPressed[4];
    // Main game loop
    while (!WindowShouldClose()){
        // Set camera target back to normal
        camera.target = (Vector2){playerData->allPlayers[id].position.x, playerData->allPlayers[id].position.y};
        // camera.zoom = Should change according to size
        
        // Updates pressed key array
        // updateKeysPressed(keysPressed);
        // Send new packet


        // Receive packets and update players data before drawing


        // Draw
        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode2D(camera);
        // Draws map
        // DrawRectangle(-(mapWidth / 2), mapHeight / 2, mapWidth, mapHeight, DARKGRAY);
        // Draws players
        //                 20 here supposed to be current online player amount, should be changed
        for(int i = 0; i < 20; i++){
            if(playerInScreen(&playerData->allPlayers[id], &playerData->allPlayers[i])){
                // Getting HEX colors to rgb format
                int red, green, blue;
                sscanf((char*)&playerData->allPlayers[i].color, "%02x%02x%02x", &red, &green, &blue);
                Color playersColor = {red, green, blue, 255};
                Vector2 playerPosition = {playerData->allPlayers[i].position.x, playerData->allPlayers[i].position.y};

                DrawCircleV(playerPosition, getRadius(&playerData->allPlayers[i]), playersColor);
                // printf("Circle position is %d x %d, and radius is %f\n", playerData->allPlayers[i].position.x, playerData->allPlayers[i].position.y, getRadius(&playerData->allPlayers[i]));

                int usernameLength = MeasureText((char*)&playerData->allPlayers[i].username, 10);
                DrawText((char*)&playerData->allPlayers[i].username, playerData->allPlayers[i].position.x - usernameLength / 2, playerData->allPlayers[i].position.y, 10, MAGENTA);
            }
        }
        // Draw food, currently no clue where the food should be so its not implemented

        EndMode2D();

        EndDrawing();
        /* Updating the camera view for the game */
        playersLocation.x = playerData->allPlayers[id].position.x;
        playersLocation.y = playerData->allPlayers[id].position.y;
        camera.target = playersLocation;
        camera.offset = (Vector2){screenWidth/2, screenHeight/2};
        camera.zoom = set_zoom_value(getRadius(&playerData->allPlayers[id]));
    }
    CloseWindow();
    // return NULL;
}

void send_client_request(int server_fd){
    int data_size = 0;
    unsigned char username_length = (unsigned char)strlen((char*)&myPlayer[0]->username);
    unsigned char* data_seg = (unsigned char*)malloc(sizeof(unsigned char) * ((int)username_length + 8));
    memcpy(data_seg, &username_length, 1);
    addByteToData(data_seg, (unsigned char)strlen((char*)&myPlayer[0]->username), &data_size);
    addStringToData(data_seg, myPlayer[0]->username, &data_size);
    addStringToData(data_seg, myPlayer[0]->color, &data_size);
    makePacket(buffer, Client_request, data_size, 0, data_seg);
    sendPacket(buffer, data_size, server_fd);
    myPlayer[0]->status = 1;
    free(data_seg);
}

void send_keyboard_presses(int server_fd, int CLIENT_PACKET_COUNTER){
    unsigned char player_data[2];
    int player_data_size = 0;
    addByteToData(player_data, (unsigned char)game_id, &player_data_size);
    addByteToData(player_data, (unsigned char)myPlayer[0]->id, &player_data_size);

    unsigned char keys_pressed[5] = {0};
    updateKeysPressed(keys_pressed);

    unsigned char data[11];
    clear_buffer(data);
    memcpy(data, player_data, sizeof(player_data));
    memcpy(data + 5, keys_pressed, sizeof(keys_pressed));
    makePacket(buffer, Player_update, 10, CLIENT_PACKET_COUNTER, data);
    sendPacket(buffer, 10, server_fd);
}

int debugging = 1;

void* server_connection_send(void* server_fd_ptr){
    int server_fd = *(int*)server_fd_ptr;
    printf("Connected to server socket:%d succesfully.\n", server_fd);
    // if(!debugging){
    //     player_init();
    // }
    // else{
    //     strcpy((char*)&myPlayer[0]->username, "USER");
    //     strcpy((char*)&myPlayer[0]->color, "FF6666");
    //     myPlayer[0]->id = -1;
    //     myPlayer[0]->status = 0;
    //     setupDone = 1;
    // }
    // while(!setupDone){}

    /* Sending client request packet */
    send_client_request(server_fd);

    for(;;){
        if(myPlayer[0]->id == -1){
            continue;
            /* Wait till allowed to play */ 
        }

        int CLIENT_PACKET_COUNTER = 0;
        while(1){
            // WRITE KEYPRESSES TO SERVER
            // Player Update should be sent every frame
            send_keyboard_presses(server_fd, CLIENT_PACKET_COUNTER);
            CLIENT_PACKET_COUNTER++;
        }
    }
}

void server_connection_read(int client_socket){
    // Implement reading of packets
    // Wait for servers game info than join the started game
    while(myPlayer[0]->status != 1){}
    unsigned char get[1];
    while(1){
        clear_buffer(readBuffer);
        int nullCounter = 0;
        int charCounter = 0;
        while(nullCounter != 4){
            read(client_socket, get, 1);
            if(get[0] == '\0'){
                nullCounter++;
            }
            readBuffer[charCounter] = get[0];
            charCounter++;
        }
        if(charCounter == 4){continue;}
        decode(readBuffer);
        int data_segment_size = getDataSize(readBuffer);
        int packetType = getPacketType(readBuffer);
        printf("Got packet type %d\n", packetType);
        unsigned char checksum_char = readBuffer[charCounter - 3];
        int received_checksum = (int)checksum_char;
        int calculated_checksum = xor_check(readBuffer, data_segment_size);
        if(received_checksum != calculated_checksum || (data_segment_size + 14 != charCounter && data_segment_size != 0)){
            print_bytes(readBuffer, charCounter);
            if(received_checksum != calculated_checksum){
                printf("Received checksum %d but calculated %d\n", received_checksum, calculated_checksum);
            }
            if(data_segment_size + 14 != charCounter){
                printf("Read %d bytes but should be %d\n", charCounter, data_segment_size + 14);
            }
            printf("Received bad packet!\n");
            exit(0);
            continue;
        }
        if(packetType == Game_state){
            printf("gamestate");
        }
        if(packetType == Approval){
            game_id = (int)readBuffer[11];
            my_id = (int)readBuffer[12];

            int *field_size_x, *field_size_y, *time_limit, *live_count, *initial_size;
            initial_size = (int*)(&readBuffer[13]);
            field_size_x = (int*)(&readBuffer[17]);
            field_size_y = (int*)(&readBuffer[21]);
            time_limit = (int*)(&readBuffer[25]);
            live_count = (int*)(&readBuffer[29]);


            playerData->allPlayers[my_id] = *myPlayer[0];
            *myPlayer = &playerData->allPlayers[my_id];
            myPlayer[0]->id = my_id;

            struct Position map_data;
            map_data.x = *field_size_x;
            map_data.y = *field_size_y;

            myPlayer[0]->position.x = *field_size_x;
            myPlayer[0]->position.y = *field_size_y;
            // int pid = fork();
            // if(pid){
            //     start_client_view((void*)&map_data, my_id);
            // }
        }
        else if(packetType == Game_state){
            // print_bytes(readBuffer, charCounter);
            // printf("This is gamestate bruh!!!\n");
            int playerAmount = (int)(readBuffer[12]);
            // printf("Detected %d players\n", playerAmount);
            int byteCounter = 13;
            int i;
            for(i = 0; i < playerAmount; i++){
                int received_players_id = (int)(readBuffer[byteCounter]);
                byteCounter += 1;
                int username_length = (int)(readBuffer[byteCounter]);
                byteCounter += 1;
                memcpy(&playerData->allPlayers[received_players_id].username, &readBuffer[byteCounter], username_length);
                byteCounter += username_length;
                int *players_x, *players_y, *players_size, *players_life;
                players_x = (int*)(&readBuffer[byteCounter]);
                byteCounter += 4;
                players_y = (int*)(&readBuffer[byteCounter]);
                byteCounter += 4;
                memcpy(&playerData->allPlayers[received_players_id].color, &readBuffer[byteCounter], 6);
                byteCounter += 6;
                playerData->allPlayers[received_players_id].color[6] = '\0';
                players_size = (int*)(&readBuffer[byteCounter]);
                byteCounter += 4;
                byteCounter += 4;
                players_life = (int*)(&readBuffer[byteCounter]);
                byteCounter += 4;

                // Update player data
                playerData->allPlayers[received_players_id].position.x = *players_x;
                playerData->allPlayers[received_players_id].position.y = *players_y;
                playerData->allPlayers[received_players_id].size = *players_size;
                playerData->allPlayers[received_players_id].lives = *players_life;
            }
            // read food data
            for(i = 0; i < FOOD_AMOUNT; i++){
                int *food_x, *food_y;
                food_x = (int*)(&readBuffer[byteCounter]);
                byteCounter += 4;
                food_y = (int*)(&readBuffer[byteCounter]);
                byteCounter += 4;
                foodInfo->foodPosition[i].position.x = *food_x;
                foodInfo->foodPosition[i].position.y = *food_y;
            }
            int time_left = (*(int*)(&readBuffer[byteCounter]));
            // += 1 so no errors
            time_left += 1;
            printf("Players location in gamestate is %d x %d\n", myPlayer[0]->position.x, myPlayer[0]->position.y);
        }
        else if(packetType == Game_lost){
            // Show game lost, replace exit
            exit(0);
        }
        else if(packetType == Game_ended){
            // Show scoreboard
            exit(0);
        }
    }
}

void start_client(char* ip_address, int port_no){
    int client_socket;
    struct sockaddr_in server_addr;
    struct hostent* address = gethostbyname(ip_address);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_no);
    //server_addr.sin_addr.s_addr = inet_addr(ip_address);
    inet_pton(AF_INET, address->h_name, &server_addr.sin_addr);

    if((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("Could not create a socket.");
        exit(0);
    }
    if(connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("Could not connect a socket!\n");
        exit(0);
    }
    pthread_t client_thread;
    if(pthread_create(&client_thread, NULL, server_connection_send, (void*)&client_socket)){
        printf("Could not create a thread\n");
        exit(0);
    }
    server_connection_read(client_socket);
}

int get_argument(int argc, char** argv, char type, char* buffer){
    int i;
    char named_argument[] = {'-', type, '=', '\0'};
    for(i = 1; i < argc; i++){
        if(strstr(argv[i], &named_argument[0])){
            strncpy(buffer, argv[i] + 3, strlen(argv[i]) - 2);
            return 1;
        }
    }
    return 0;
}
