#include "raylib.h"
#include "rlgl.h"

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
    int direction;
} Shark;

typedef struct SeaCreature {
    Vector2 position; // current positioning
    Vector2 origin; // helps determine which direction to head when spawned
    bool active;
    int direction;
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
int playerDirection = 1; // 1 = left, -1 = right
int FishSpawnTimer = 0;
Shark mrShark;
int sharkBounces = 0;
int sharkMaxBounces = 5;
int score = 0;
int SharkSpawnTimer = 0;
int sharkDirection = 1; // 1 = left, -1 = right
bool pause, GameOver = false;
SeaCreature creatures[27];
Vector2 creatureSizes[9] = { {30,25}, {30,35}, {30,20}, {31,23}, {40,32}, {20,35}, {30,30}, {30,30}, {30,60} }; // use SC type to get size
float creatureSpeed[9] = {1, 1.3, 1.5, 1.7, 2, 2, 2, 2, 2}; // use SC type to get speed
int creatureRank[9] = {1, 2, 3, 4, 5, 5, 5, 5, 6}; // use SC type to get rank. Rank determines what a creature can eat. (jellyfish are immune)

void SetShark() {
    mrShark.active = true;
    mrShark.objective = playerPosition;
    mrShark.width = 120;
    mrShark.height = 60;

    float module = sqrt(pow(mrShark.objective.x - mrShark.position.x, 2) + pow(mrShark.objective.y - mrShark.position.y, 2));

    float sideX = (mrShark.objective.x - mrShark.position.x) / module;
    float sideY = (mrShark.objective.y - mrShark.position.y) / module;

    if (mrShark.position.x <= 5)
        sharkDirection = -1;
    else
        sharkDirection = 1;

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
            int pickSide[2] = {20,(float)screenWidth - 20};
            float pickHeight = GetRandomNum(20,screenHeight - 50);
            float ps = pickSide[rand()%2];
            if (creatures[pickCreature].type == 6 || creatures[pickCreature].type == 7) pickHeight = (float)screenHeight - 40;
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
            if (creatures[i].origin.x <= 40)
                creatures[i].position.x = creatures[i].position.x + creatureSpeed[creatures[i].type];
            else if (creatures[i].origin.x >= (float)screenWidth - 40)
                creatures[i].position.x = creatures[i].position.x - creatureSpeed[creatures[i].type];
            // de-spawn
            if ((creatures[i].origin.x >= (float)screenWidth && creatures[i].position.x <= 0) ||
                (creatures[i].origin.x <= 0 && creatures[i].position.x >= (float)screenWidth ))
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

    RenderTexture2D Fish1Texture_Left = LoadRenderTexture(30, 25);
    RenderTexture2D Fish1Texture_Right = LoadRenderTexture(30, 25);
    RenderTexture2D Fish2Texture_Left = LoadRenderTexture(30, 35);
    RenderTexture2D Fish2Texture_Right = LoadRenderTexture(30, 35);
    RenderTexture2D SharkTexture = LoadRenderTexture(120, 60);
    RenderTexture2D CrustaceanTexture = LoadRenderTexture(40, 40);

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        struct Rectangle playerRec = { playerPosition.x, playerPosition.y, creatureSizes[playerRank].x, creatureSizes[playerRank].y };

        // these should flip depending on which direction shark is facing
        struct Rectangle sharkTailRec = { mrShark.position.x + (mrShark.width / 2), mrShark.position.y, mrShark.width / 2, mrShark.height };
        struct Rectangle sharkBiteRec = { mrShark.position.x, mrShark.position.y, mrShark.width / 2, mrShark.height };
        if (sharkDirection == -1){
            sharkTailRec = (Rectangle){ mrShark.position.x, mrShark.position.y, mrShark.width / 2, mrShark.height };
            sharkBiteRec = (Rectangle){ mrShark.position.x + (mrShark.width / 2), mrShark.position.y, mrShark.width / 2, mrShark.height };
        }

        if (IsKeyPressed(KEY_P)) { if (pause) pause = false; else pause = true; }
        if (IsKeyPressed(KEY_ENTER) && GameOver) { SetVars(); printf("restarting game"); }

        if (!pause && !GameOver) {
            if (IsKeyDown(KEY_RIGHT) && playerPosition.x < screenWidth) { playerPosition.x += 2.0f; playerDirection = -1; }
            if (IsKeyDown(KEY_LEFT) && playerPosition.x > 0) { playerPosition.x -= 2.0f; playerDirection = 1; }
            if (IsKeyDown(KEY_UP) && playerPosition.y > 0) playerPosition.y -= 2.0f;
            if (IsKeyDown(KEY_DOWN) && playerPosition.y < screenHeight) playerPosition.y += 2.0f;
            
            SharkRoam();
            FishSpawn();
            FishMoveAndDeSpawn();
        }

        // CREATE SPRITES
        BeginTextureMode(Fish1Texture_Left);
            //rlDisableBackfaceCulling(); // useful if we draw an object backwards and don't know why it isn't working
            DrawTriangle((Vector2) { 20, 0 }, (Vector2) { 0, 12 }, (Vector2) { 20, 25 }, YELLOW); // body
            DrawTriangle((Vector2) { 30, 5 }, (Vector2) { 15, 12 }, (Vector2) { 30, 16 }, YELLOW); // tail
        EndTextureMode();
        BeginTextureMode(Fish1Texture_Right);
            DrawTriangle((Vector2) { 0, 5 }, (Vector2) { 0, 16 }, (Vector2) { 15, 12 }, YELLOW); // tail
            DrawTriangle((Vector2) { 10, 0 }, (Vector2) { 10, 25 }, (Vector2) { 30, 12 }, YELLOW); // body
        EndTextureMode();
        BeginTextureMode(Fish2Texture_Left);
            //rlDisableBackfaceCulling(); // useful if we draw an object backwards and don't know why it isn't working
            DrawTriangle((Vector2) { 20, 0 }, (Vector2) { 0, 17 }, (Vector2) { 20, 30 }, YELLOW); // body
            DrawTriangle((Vector2) { 30, 5 }, (Vector2) { 15, 17 }, (Vector2) { 30, 21 }, YELLOW); // tail
        EndTextureMode();
        BeginTextureMode(Fish2Texture_Right);
            DrawTriangle((Vector2) { 0, 5 }, (Vector2) { 0, 21 }, (Vector2) { 15, 17 }, YELLOW); // tail
            DrawTriangle((Vector2) { 10, 0 }, (Vector2) { 10, 30 }, (Vector2) { 30, 17 }, YELLOW); // body
        EndTextureMode();
        BeginTextureMode(SharkTexture);
            ClearBackground(BLANK);
            if (sharkDirection == 1) {
                DrawRectangleRounded((Rectangle) { 0, 15, 110, 30 }, 1, 1, BLACK); // body
                DrawTriangle((Vector2) { 60, 5 }, (Vector2) { 15, 30 }, (Vector2) { 60, 60 }, BLACK); // fins
                DrawTriangle((Vector2) { 115, 10 }, (Vector2) { 70, 25 }, (Vector2) { 115, 60 }, BLACK); // tail
                //DrawRectangle(0 + (mrShark.width / 2), 0, mrShark.width / 2, mrShark.height, RED); // test
            }
            else {
                rlDisableBackfaceCulling();
                DrawRectangleRounded((Rectangle) { 10, 15, 110, 30 }, 1, 1, BLACK); // body
                DrawTriangle((Vector2) { 60, 5 }, (Vector2) { 105, 30 }, (Vector2) { 60, 60 }, BLACK); // fins
                DrawTriangle((Vector2) { 5, 10 }, (Vector2) { 50, 25 }, (Vector2) { 5, 60 }, BLACK); // tail
            }
        EndTextureMode();
        BeginTextureMode(CrustaceanTexture);
            DrawRing((Vector2){10, 40}, 2, 5, 90, 270, 2, ORANGE);
            DrawRing((Vector2){25, 40}, 2, 5, 90, 270, 2, ORANGE);
            DrawLine(10, 35, 10, 0, ORANGE);
            DrawLine(25, 35, 25, 0, ORANGE);
            DrawRectangleRounded((Rectangle) { 8, 5, 19, 25 }, 10, 10, YELLOW); // body
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
            if (CheckCollisionRecs(playerRec, sharkBiteRec)) {
                if (!GameOver && !pause)
                    lives--;
                playerPosition = (Vector2){0,0};
                if (lives < 0) {
                    GameOver = true;
                }
            } else if (CheckCollisionRecs(playerRec, sharkTailRec)) {
                mrShark.active = false;
            }

            if (playerDirection == 1)
                DrawTexture(Fish1Texture_Left.texture, playerPosition.x, playerPosition.y, YELLOW); // draw player
            else
                DrawTexture(Fish1Texture_Right.texture, playerPosition.x, playerPosition.y, YELLOW);

            if (mrShark.active)
                DrawTexture(SharkTexture.texture, mrShark.position.x, mrShark.position.y, WHITE); // draw shark
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

                    if (creatures[i].type == 6 || creatures[i].type == 7)
                        DrawTexture(CrustaceanTexture.texture, creatures[i].position.x, creatures[i].position.y, ORANGE);
                    else {
                        if (creatures[i].origin.x <= 20)
                            DrawTexture(Fish1Texture_Right.texture, creatures[i].position.x, creatures[i].position.y, FishColor(creatures[i].type));
                        else
                            DrawTexture(Fish1Texture_Left.texture, creatures[i].position.x, creatures[i].position.y, FishColor(creatures[i].type));
                    }
                    
                }
            }
        EndDrawing();
    }
    UnloadRenderTexture(Fish1Texture_Left);
    UnloadRenderTexture(Fish1Texture_Right);
    UnloadRenderTexture(SharkTexture);
    UnloadRenderTexture(CrustaceanTexture);
    CloseWindow();
    return 0;
}

