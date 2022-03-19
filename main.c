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
    int type; // 1-5 = fish ranked, 6 = seahorse, 7 = crab, 8 = lobster, 9 = jellyfish (use for creatureSize and creatureSpeed)
} SeaCreature;

const int screenWidth = 800;
const int screenHeight = 450;
Vector2 playerPosition;
int FishSpawnTimer = 0;
Shark mrShark;
int sharkBounces = 0;
int sharkMaxBounces = 5;
int score = 0;
SeaCreature creatures[27];
Vector2 creatureSizes[9] = { {20,15}, {23,20}, {30,20}, {31,23}, {40,32}, {20,35}, {30,30}, {30,30}, {30,60} }; // use SC type to get size
float creatureSpeed[9] = {1, 1.3, 1.5, 1.7, 2, 2, 2, 2, 2}; // use SC type to get speed
int creatureRank[9] = {1, 2, 3, 4, 5, 5, 5, 5, 6}; // use SC type to get rank. Rank determines what a creature can eat. (jellyfish are immune)
Color creatureColors[9] = {DARKBLUE, GOLD, PURPLE, YELLOW, RED, GREEN, ORANGE, ORANGE, PINK};
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

void SetFish() {
    int t = 1; // creature type
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
    playerPosition.x = (float)screenWidth / 2;
    playerPosition.y = (float)screenHeight / 2;

    mrShark.position = (Vector2){ (float)screenWidth - 20, 20 };

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

        //printf("DEBUG: shark coords x:%f y:%f sx:%f sy:%f\n", mrShark.position.x, mrShark.position.y, mrShark.speed.x, mrShark.speed.y);
    }
}

int GetRandomNum(int min, int max)
{
    int range, result, cutoff;
    if (min >= max) return min; 
    range = max - min + 1;
    cutoff = (RAND_MAX / range) * range;
    do { result = rand(); } while (result >= cutoff);
    return result % range + min;
}

int main(void)
{
    // Initialization
    InitWindow(screenWidth, screenHeight, "Shark! Shark!");
    SetVars();
    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        if (IsKeyDown(KEY_RIGHT) && playerPosition.x < screenWidth) playerPosition.x += 2.0f;
        if (IsKeyDown(KEY_LEFT) && playerPosition.x > 0) playerPosition.x -= 2.0f;
        if (IsKeyDown(KEY_UP) && playerPosition.y > 0) playerPosition.y -= 2.0f;
        if (IsKeyDown(KEY_DOWN) && playerPosition.y < screenHeight) playerPosition.y += 2.0f;

        struct Rectangle playerRec = { playerPosition.x, playerPosition.y, 20, 15 };
        //struct Rectangle sharkRec = { sharkPosition.x, sharkPosition.y, mrShark.width, mrShark.height };
        
        SharkRoam();

        if (FishSpawnTimer >= 50){
            int pickCreature = GetRandomNum(0,26);
            if (!creatures[pickCreature].active){
                srand ( time(NULL) );
                int pickSide[] = {0,(float)screenWidth - 20};
                int pickHeight = GetRandomNum(20,screenHeight - 50);
                if (creatures[pickCreature].type == 7 || creatures[pickCreature].type == 8) pickHeight = (float)screenHeight - creatureSizes[7].x;
                creatures[pickCreature].origin = (Vector2){pickSide[rand()%2],pickHeight};
                creatures[pickCreature].position = creatures[pickCreature].origin;
                creatures[pickCreature].active = true;
            }
            FishSpawnTimer = 0;
        }
        FishSpawnTimer++;

        // make fish move or de-spawn when destination reached
        for (int i = 0; i < 27; i++){
            if (creatures[i].active) {
                // move
                if (creatures[i].origin.x == 0)
                    creatures[i].position.x = creatures[i].position.x + creatureSpeed[creatures[i].type];
                else if (creatures[i].origin.x == (float)screenWidth - 20)
                    creatures[i].position.x = creatures[i].position.x - creatureSpeed[creatures[i].type];
                // de-spawn
                if ((creatures[i].origin.x == (float)screenWidth - 20 && creatures[i].position.x <= 0) ||
                    (creatures[i].origin.x == 0 && creatures[i].position.x >= (float)screenWidth - 20))
                    creatures[i].active = false;
            }
        }

        // DRAW
        BeginDrawing();
            ClearBackground(BLUE);
            DrawText(TextFormat("SCORE %4i", score), 10, 10, 20, RAYWHITE);
            /*if (CheckCollisionRecs(playerRec, mrShark.position)) {
                DrawText("GAME OVER!", screenWidth / 2, screenHeight / 2, 20, RED);
            }*/
            DrawRectangle(playerPosition.x, playerPosition.y, 20, 15, YELLOW);
            if (mrShark.active)
                DrawRectangle(mrShark.position.x, mrShark.position.y, mrShark.width, mrShark.height, BLACK);
            for (int i = 0; i < 27; i++){
                if (creatures[i].active){
                    DrawRectangle(creatures[i].position.x, creatures[i].position.y, creatureSizes[creatures[i].type].x, creatureSizes[creatures[i].type].y, creatureColors[i]);
                }
            }
        EndDrawing();
    }
    CloseWindow();        // Close window and OpenGL context
    return 0;
}

