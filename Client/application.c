#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "client.h"
#include "raylib.h"

bool IsAnyKeyPressed(){
    bool keyPressed = false;
    int key = GetKeyPressed();
    if ((key >= 32) && (key <= 126)) keyPressed = true;
    return keyPressed;
}

void initialize_client_connection(int argc, char** argv){
    // foodInfo = mmap(NULL, sizeof(struct Food) * FOOD_AMOUNT, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    // playerData = mmap(NULL, sizeof(struct Player) * MAX_CLIENTS, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    // myPlayer = mmap(NULL, sizeof(struct Player*), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    // myPlayer[0] = mmap(NULL, sizeof(struct Player), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    char* ip_address = malloc(128);
    int port_no = 0;
    char buffer[BUFF_SIZE];
    if(get_argument(argc, argv, 'p', buffer)){
        port_no = atoi(buffer);
    }
    if(get_argument(argc, argv, 'a', buffer)){
        strcpy(ip_address, buffer);
    }
    if(!port_no || !ip_address ){
        printf("Incorrect argument list.\n");
        printf("Example: %s -p=<port> -a=<ip address>\n", argv[0]);
        exit(0);
    }
    start_client(ip_address, port_no);
}

int main(int argc, char** argv){
    foodInfo = mmap(NULL, sizeof(struct Food) * FOOD_AMOUNT, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    playerData = mmap(NULL, sizeof(struct Player) * MAX_CLIENTS, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    myPlayer = mmap(NULL, sizeof(struct Player*), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    myPlayer[0] = mmap(NULL, sizeof(struct Player), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    
    int MAX_INPUT_CHARS = 255;
    const int screenWidth = 960;
    const int screenHeight = 500;
    InitWindow(screenWidth, screenHeight, "Agar.io clone");

    /* Setting up registering screen*/
    unsigned char* name = &myPlayer[0]->username[0];
    int letterCount = 0;
    int entrCounter = 0;

    char curretText[100] = "Enter you username!";
    Rectangle textBox = {screenWidth/2 - 100, 180, 225, 50 };
    bool mouseOnText = false;
    int framesCounter = 0;

    SetTargetFPS(60);

    // struct Position* map_parameters = (struct Position*)map_info;
    int mapWidth;
    int mapHeight;
    Vector2 playersLocation;
    Camera2D camera = {};
    // camera.target = playersLocation;
    // camera.offset = (Vector2){screenWidth/2, screenHeight/2};
    // camera.zoom = set_zoom_value(getRadius(myPlayer[0]));

    while (!WindowShouldClose()){
        if(entrCounter != 2){
            if (CheckCollisionPointRec(GetMousePosition(), textBox)) mouseOnText = true;
            else mouseOnText = false;

            if (mouseOnText){
                int key = GetKeyPressed();
                while (key > 0){
                    if ((key >= 32) && (key <= 125) && (letterCount < MAX_INPUT_CHARS)){
                        name[letterCount] = (char)key;
                        letterCount++;
                    }
                    key = GetKeyPressed();
                }
                if (IsKeyPressed(KEY_BACKSPACE)){
                    letterCount--;
                    name[letterCount] = '\0';
                    if (letterCount < 0) letterCount = 0;
                }
                if(IsKeyPressed(KEY_ENTER)){
                    strcpy(curretText, "Pick players HEX color: #");
                    entrCounter++;
                    MAX_INPUT_CHARS = 6;
                    name = &myPlayer[0]->color[0];
                }
            }
            if (mouseOnText) framesCounter++;
            else framesCounter = 0;
            BeginDrawing();
                ClearBackground(RAYWHITE);

                DrawText(curretText, 240, 140, 20, GRAY);

                if (mouseOnText) DrawRectangleLines(textBox.x, textBox.y, textBox.width, textBox.height, RED);
                else DrawRectangleLines(textBox.x, textBox.y, textBox.width, textBox.height, DARKGRAY);

                DrawText((char*)name, textBox.x + 5, textBox.y + 8, 20, MAROON);
                DrawText(TextFormat("INPUT CHARS: %i/%i", letterCount, MAX_INPUT_CHARS), 315, 250, 20, DARKGRAY);

            EndDrawing();
        }
        if(entrCounter == 2){
                myPlayer[0]->id = -1;
                myPlayer[0]->status = 0;
                setupDone = 1;
                entrCounter++;
                initialize_client_connection(argc, argv);
        }
        else if(myPlayer[0]->position.x){
            if(!mapWidth){
                mapWidth = myPlayer[0]->position.x;
                mapHeight = myPlayer[0]->position.y;
            }
            // Set camera target back to normal
            camera.target = (Vector2){myPlayer[0]->position.x, myPlayer[0]->position.y};
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
            DrawRectangle(-(mapWidth / 2), mapHeight / 2, mapWidth, mapHeight, DARKGRAY);
            // Draws players
            //                 20 here supposed to be current online player amount, should be changed
            for(int i = 0; i < 20; i++){
                if(playerInScreen(myPlayer[0], &playerData->allPlayers[i])){
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
            playersLocation.x = myPlayer[0]->position.x;
            playersLocation.y = myPlayer[0]->position.y;
            camera.target = playersLocation;
            camera.offset = (Vector2){screenWidth/2, screenHeight/2};
            camera.zoom = set_zoom_value(getRadius(myPlayer[0]));
        }
    }

    CloseWindow();

    return 0;
}
