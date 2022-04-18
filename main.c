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
int playerRank = 0;
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
float creatureSpeed[9] = {1, 1.3, 1.5, 1.7, 2, 2, 2, 2, 2}; // use SC type to get speed
int creatureRank[9] = {1, 2, 3, 4, 5, 5, 5, 5, 6}; // use SC type to get rank. Rank determines what a creature can eat. (jellyfish are immune)

void SetShark() {
    mrShark.active = true;
    mrShark.objective = playerPosition;

    float module = sqrt(pow(mrShark.objective.x - mrShark.position.x, 2) + pow(mrShark.objective.y - mrShark.position.y, 2));

    float sideX = (mrShark.objective.x - mrShark.position.x) / module;
    float sideY = (mrShark.objective.y - mrShark.position.y) / module;

    if (mrShark.position.x <= 5)
        sharkDirection = -1;
    else
        sharkDirection = 1;

    mrShark.speed = (Vector2){ sideX, sideY };
}

void SetFish() {
    int t = 0; // creature type
    for (int i = 0; i < 27; i++){ // generate 27 sea creatures
        SeaCreature sc;
        sc.position = (Vector2) {0,0};
        sc.origin = (Vector2) {0,0};
        if (t > 8)
            t = 8;
        sc.type = t;
        sc.active = false;
        creatures[i] = sc;
        if (i%3 == 0) t++; // 3 of every type
    }
}

void SetVars() {
    score = 0;
    playerRank = 0;
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
            if (sharkBounces == sharkMaxBounces) {
                mrShark.position.x = -10;
                mrShark.position.y = -10;
                mrShark.active = false;
            }
            sharkBounces++;
        }
    } else {
        if (SharkSpawnTimer >= 600) {
            mrShark.active = true;
            mrShark.position = (Vector2){ (float)screenWidth - 20, 20 };
            SharkSpawnTimer = 0;
        }
        SharkSpawnTimer++;
    }
}

void FishSpawn() {
    if (FishSpawnTimer >= 60){
        int pickCreature = 0;
        pickCreature = GetRandomNum(0,26);
        if (!creatures[pickCreature].active){
            srand ( time(NULL) );
            creatures[pickCreature].active = true;
            int pickSide[2] = {20,(float)screenWidth - 20};
            float pickHeight = GetRandomNum(20,screenHeight - 50);
            float ps = pickSide[rand()%2];
            if (creatures[pickCreature].type == 6 || creatures[pickCreature].type == 7) pickHeight = (float)screenHeight - 40;
            creatures[pickCreature].origin = (Vector2){ps,pickHeight};
            creatures[pickCreature].position = (Vector2){ps,pickHeight};
            printf("DEBUG: Spawning Fish coords x:%f y:%f type:%i active:%i\n", ps, pickHeight, creatures[pickCreature].type, creatures[pickCreature].active);
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
            if ((creatures[i].origin.x > (float)screenWidth && creatures[i].position.x < 0) ||
                (creatures[i].origin.x < 0 && creatures[i].position.x > (float)screenWidth )) {
                    creatures[i].position.x = -10;
                    creatures[i].position.y = -10;
                    creatures[i].active = false;
                    printf("DEBUG: DE-Spawning Fish active:%i\n", creatures[i].active);
                }
        }
    }
}

int main(void)
{
    // Initialization
    InitWindow(screenWidth, screenHeight, "Shark! Shark!");
    SetVars();
    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

    Texture2D FishTexturesRight[5] = { LoadTexture("resources/rank1.png"), LoadTexture("resources/rank2.png"), LoadTexture("resources/rank3.png"), LoadTexture("resources/rank4.png"), LoadTexture("resources/rank5.png") };
    Image FishImagesLeft[5] = { LoadImage("resources/rank1.png"), LoadImage("resources/rank2.png"), LoadImage("resources/rank3.png"), LoadImage("resources/rank4.png"), LoadImage("resources/rank5.png") };
    ImageFlipHorizontal(&FishImagesLeft[0]);
    ImageFlipHorizontal(&FishImagesLeft[1]);
    ImageFlipHorizontal(&FishImagesLeft[2]);
    ImageFlipHorizontal(&FishImagesLeft[3]);
    ImageFlipHorizontal(&FishImagesLeft[4]);
    Texture2D FishTexturesLeft[5] = { LoadTextureFromImage(FishImagesLeft[0]), LoadTextureFromImage(FishImagesLeft[1]), LoadTextureFromImage(FishImagesLeft[2]), LoadTextureFromImage(FishImagesLeft[3]), LoadTextureFromImage(FishImagesLeft[4]) };
    Texture2D SharkTexture = LoadTexture("resources/shark.png");
    Image SharkImage = LoadImage("resources/shark.png");
    ImageFlipHorizontal(&SharkImage);
    Texture2D SharkTextureLeft = LoadTextureFromImage(SharkImage);
    Texture2D SharkDeadTexture = LoadTexture("resources/shark_dead.png");
    Texture2D SeaHorseTexture = LoadTexture("resources/seahorse.png");
    Texture2D CrabTexture = LoadTexture("resources/crab.png");
    Texture2D LobsterTexture = LoadTexture("resources/lobster.png");
    Texture2D JellyfishTexture = LoadTexture("resources/jellyfish.png");

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        struct Rectangle playerRec = { playerPosition.x, playerPosition.y, 16, 16 };

        // these should flip depending on which direction shark is facing
        struct Rectangle sharkTailRec = { mrShark.position.x + (SharkImage.width * 2 / 2), mrShark.position.y, SharkImage.width * 2 / 2, SharkImage.height * 2 };
        struct Rectangle sharkBiteRec = { mrShark.position.x, mrShark.position.y, SharkImage.width / 2, SharkImage.height * 2 };
        if (sharkDirection == -1){
            sharkTailRec = (Rectangle){ mrShark.position.x, mrShark.position.y, SharkImage.width * 2 / 2, SharkImage.height * 2 };
            sharkBiteRec = (Rectangle){ mrShark.position.x + (SharkImage.width * 2 / 2), mrShark.position.y, SharkImage.width * 2 / 2, SharkImage.height * 2 };
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
                mrShark.position.x = -10;
                mrShark.position.y = -10;
                mrShark.active = false;
                score = score + 100;
            }

            Vector2 PlayerGoTo = { playerPosition.x, playerPosition.y };
            if (playerDirection == 1) {
                //DrawTexture(FishTextures[0], playerPosition.x, playerPosition.y, YELLOW); // draw player
                DrawTextureEx(FishTexturesLeft[playerRank], PlayerGoTo, 0, 2, YELLOW);
            } 
            else {
                DrawTextureEx(FishTexturesRight[playerRank], PlayerGoTo, 0, 2, YELLOW);
            }

            if (mrShark.active) {
                    Vector2 GoTo = { mrShark.position.x, mrShark.position.y };
                    if (sharkDirection == 1) // left
                        DrawTextureEx(SharkTextureLeft, GoTo, 0, 2, YELLOW);
                    else // right
                        DrawTextureEx(SharkTexture, GoTo, 0, 2, YELLOW);
            }
            for (int i = 0; i < 27; i++){
                if (creatures[i].active){
                    if (creatures[i].type < 0 || creatures[i].type > 8) continue;
                    struct Rectangle FishRec = { creatures[i].position.x, creatures[i].position.y, 16, 16 };
                    if (CheckCollisionRecs(playerRec, FishRec)) {
                        if ((playerRank + 1) >= creatureRank[creatures[i].type]) {
                            creatures[i].position.y = -10;
                            creatures[i].position.x = -10;
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
                        DrawTextureEx(CrabTexture, (Vector2) { creatures[i].position.x, creatures[i].position.y }, 0, 2, YELLOW);
                    else if (creatures[i].type <= 4 && creatures[i].type >= 0) {
                        Vector2 GoTo = { creatures[i].position.x, creatures[i].position.y };
                        if (creatures[i].origin.x <= 20)
                            DrawTextureEx(FishTexturesRight[creatures[i].type], GoTo, 0, 2, YELLOW);
                            //DrawTexture(FishTextures[creatures[i].type], creatures[i].position.x, creatures[i].position.y, YELLOW); // RIGHT
                        else
                            DrawTextureEx(FishTexturesLeft[creatures[i].type], GoTo, 0, 2, YELLOW);
                            //DrawTexture(FishTextures[creatures[i].type], creatures[i].position.x, creatures[i].position.y, YELLOW); // LEFT
                    }
                    
                }
            }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}

