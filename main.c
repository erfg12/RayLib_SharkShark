#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

typedef struct Shark {
    Vector2 position;
    Vector2 objective;
    Vector2 speed;
    int width;
    int height;
    bool active;
} Shark;

typedef struct SeaCreature {
    Vector2 position; // current positioning
    Vector2 origin; // helps determine which direction to head when spawned
    bool active;
    int type; // 0-4 = fish ranked, 5 = seahorse, 6 = crab, 7 = lobster, 8 = jellyfish (use for creatureSize and creatureSpeed)
} SeaCreature;

int GetRandomNum(int min, int max)
{
    int range, result, cutoff;
    if (min >= max) return min; 
    range = max - min + 1;
    cutoff = (RAND_MAX / range) * range;
    do { result = rand(); } while (result >= cutoff);
    return result % range + min;
}

const int screenWidth = 800;
const int screenHeight = 450;
Vector2 playerPosition;
int lives = 5;
int playerRank = 1;
int FishSpawnTimer = 0;
Shark mrShark;
int sharkBounces = 0;
int sharkMaxBounces = 5;
int score = 0;
int SharkSpawnTimer = 0;
bool pause, GameOver = false;
SeaCreature creatures[27];
Vector2 creatureSizes[9] = { {20,15}, {23,20}, {30,20}, {31,23}, {40,32}, {20,35}, {30,30}, {30,30}, {30,60} }; // use SC type to get size
float creatureSpeed[9] = {1, 1.3, 1.5, 1.7, 2, 2, 2, 2, 2}; // use SC type to get speed
int creatureRank[9] = {1, 2, 3, 4, 5, 5, 5, 5, 6}; // use SC type to get rank. Rank determines what a creature can eat. (jellyfish are immune)

void SetShark() {
    mrShark.active = true;
    mrShark.objective = playerPosition;
    mrShark.width = 50;
    mrShark.height = 30;

    float module = sqrt(pow(mrShark.objective.x - mrShark.position.x, 2) + pow(mrShark.objective.y - mrShark.position.y, 2));

    float sideX = (mrShark.objective.x - mrShark.position.x) / module;
    float sideY = (mrShark.objective.y - mrShark.position.y) / module;

    mrShark.speed = (Vector2){ sideX, sideY };
}

Color FishColor(int i) {
    Color creatureColors[9] = { DARKBLUE, GOLD, PURPLE, YELLOW, RED, GREEN, ORANGE, ORANGE, PINK };
    return creatureColors[i];
}

void SetFish() {
    int t = 0; // creature type
    for (int i = 0; i < 27; i++){ // generate 27 sea creatures
        SeaCreature sc;
        sc.position = (Vector2) {0,0};
        sc.origin = (Vector2) {0,0};
        sc.type = t;
        sc.active = false;
        creatures[i] = sc;
        if (i%3 == 0) t++; // 3 of every type
    }
}

void SetVars() {
    score = 0;
    playerRank = 1;
    playerPosition.x = (float)screenWidth / 2;
    playerPosition.y = (float)screenHeight / 2;

    mrShark.position = (Vector2){ (float)screenWidth - 20, 20 };

    pause = false;
    GameOver = false;
    lives = 5;
    SetShark();
    SetFish();
    //printf("DEBUG: SETTING shark coords x:%f y:%f sx:%f sy:%f\n", mrShark.position.x, mrShark.position.y, mrShark.speed.x, mrShark.speed.y);
    //printf("DEBUG: SETTING player coords x:%f y:%f\n", playerPosition.x, playerPosition.y);
}

void SharkRoam() {
    if (mrShark.active)
    {
        mrShark.objective = playerPosition;
        mrShark.position.x += mrShark.speed.x;
        mrShark.position.y += mrShark.speed.y;

        if (mrShark.position.x <= 0 || mrShark.position.x >= (float)screenWidth - 20) {
            printf("reset position\n");
            SetShark();
            if (sharkBounces == sharkMaxBounces)
                mrShark.active = false;
            sharkBounces++;
        }
    } else {
        if (SharkSpawnTimer >= 600) {
            mrShark.position = (Vector2){ (float)screenWidth - 20, 20 };
            mrShark.active = true;
            SharkSpawnTimer = 0;
        }
        SharkSpawnTimer++;
    }
}

void FishSpawn() {
    if (FishSpawnTimer >= 120){
        int pickCreature = GetRandomNum(0,26);
        if (!creatures[pickCreature].active){
            srand ( time(NULL) );
            int pickSide[2] = {0,(float)screenWidth - 20};
            float pickHeight = GetRandomNum(20,screenHeight - 50);
            float ps = pickSide[rand()%2];
            if (creatures[pickCreature].type == 6 || creatures[pickCreature].type == 7) pickHeight = (float)screenHeight - 30;
            creatures[pickCreature].origin = (Vector2){ps,pickHeight};
            creatures[pickCreature].position = (Vector2){ps,pickHeight};
            creatures[pickCreature].active = true;
        }
        FishSpawnTimer = 0;
    }
    FishSpawnTimer++;
}

void FishMoveAndDeSpawn() {
    for (int i = 0; i < 27; i++){
        if (creatures[i].active) {
            // move
            if (creatures[i].origin.x <= 0)
                creatures[i].position.x = creatures[i].position.x + creatureSpeed[creatures[i].type];
            else if (creatures[i].origin.x >= (float)screenWidth - 20)
                creatures[i].position.x = creatures[i].position.x - creatureSpeed[creatures[i].type];
            // de-spawn
            if ((creatures[i].origin.x >= (float)screenWidth - 20 && creatures[i].position.x <= 0) ||
                (creatures[i].origin.x <= 0 && creatures[i].position.x >= (float)screenWidth - 20))
                creatures[i].active = false;
        }
    }
}

int main(void)
{
    // Initialization
    InitWindow(screenWidth, screenHeight, "Shark! Shark!");
    SetVars();
    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

    RenderTexture2D target = LoadRenderTexture(50, 50);

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        struct Rectangle playerRec = { playerPosition.x, playerPosition.y, creatureSizes[playerRank].x, creatureSizes[playerRank].y };
        struct Rectangle sharkRec = { mrShark.position.x, mrShark.position.y, mrShark.width, mrShark.height };

        if (IsKeyPressed(KEY_P)) { if (pause) pause = false; else pause = true; }
        if (IsKeyPressed(KEY_ENTER) && GameOver) { SetVars(); printf("restarting game"); }

        if (!pause && !GameOver) {
            if (IsKeyDown(KEY_RIGHT) && playerPosition.x < screenWidth) playerPosition.x += 2.0f;
            if (IsKeyDown(KEY_LEFT) && playerPosition.x > 0) playerPosition.x -= 2.0f;
            if (IsKeyDown(KEY_UP) && playerPosition.y > 0) playerPosition.y -= 2.0f;
            if (IsKeyDown(KEY_DOWN) && playerPosition.y < screenHeight) playerPosition.y += 2.0f;
            
            SharkRoam();
            FishSpawn();
            FishMoveAndDeSpawn();
        }

        // CREATE SPRITES (tested and works!)
        BeginTextureMode(target);
            ClearBackground(RAYWHITE);  // Clear render texture background color
            DrawRectangle(0, 0, 50, 50, YELLOW);
        EndTextureMode();

        // DRAW
        BeginDrawing();
            ClearBackground(BLUE);
            DrawText(TextFormat("SCORE %4i", score), 10, 10, 20, RAYWHITE);
            DrawText(TextFormat("%4i LIVES", lives), (float)screenWidth - 120, 10, 20, RAYWHITE);
            if (GameOver) 
                DrawText(TextFormat("GAME OVER!\n\nYOUR SCORE: %4i\n\nPRESS ENTER TO RESTART GAME", score), screenWidth / 2 - 100, screenHeight / 2 - 100, 20, RED);
            if (pause)
                DrawText("PAUSED\n\nPRESS P TO RESUME", screenWidth / 2 - 50, screenHeight / 2 - 50, 20, WHITE);
            if (CheckCollisionRecs(playerRec, sharkRec)) { // TO-DO: Make collision on tail hurt shark, on front kill player
                if (!GameOver && !pause)
                    lives--;
                playerPosition = (Vector2){0,0};
                if (lives < 0) {
                    GameOver = true;
                }
            }
            //DrawRectangle(playerPosition.x, playerPosition.y, creatureSizes[playerRank].x, creatureSizes[playerRank].y, YELLOW);
            DrawTexture(target.texture, playerPosition.x, playerPosition.y, YELLOW);
            if (mrShark.active)
                DrawRectangle(mrShark.position.x, mrShark.position.y, mrShark.width, mrShark.height, BLACK);
            for (int i = 0; i < 27; i++){
                if (creatures[i].active){
                    struct Rectangle FishRec = { creatures[i].position.x, creatures[i].position.y, creatureSizes[creatures[i].type].x, creatureSizes[creatures[i].type].y };
                    if (CheckCollisionRecs(playerRec, FishRec)) {
                        if (playerRank >= creatureRank[creatures[i].type]) {
                            creatures[i].active = false;
                            score = score + 100;
                            if (score%1000 == 0){
                                playerRank++;
                                printf("player rank is now %i\n", playerRank);
                            }
                        } else {
                            lives--;
                            playerPosition = (Vector2){0,0};
                            if (lives < 0) {
                                GameOver = true;
                            }
                        }
                    }
                    DrawRectangle(creatures[i].position.x, creatures[i].position.y, creatureSizes[creatures[i].type].x, creatureSizes[creatures[i].type].y, FishColor(creatures[i].type));
                }
            }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}

