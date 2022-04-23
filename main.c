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
    bool jump;
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
int SharkHurtTimer = 0;
int SharkHealth = 10;
int sharkDirection = 1; // 1 = left, -1 = right
bool pause, GameOver, playerDead, sharkBitten = false;
SeaCreature creatures[27];
float creatureSpeed[9] = {1, 1.3, 1.5, 1.7, 2, 0.8, 0.8, 2, 1.3}; // use SC type to get speed
int creatureRank[9] = {1, 2, 3, 4, 5, 5, 5, 5, 6}; // use SC type to get rank. Rank determines what a creature can eat. (jellyfish are immune)

void SetShark() {
    mrShark.active = true;
    mrShark.objective = playerPosition;

    float module = sqrt(pow(mrShark.objective.x - mrShark.position.x, 2) + pow(mrShark.objective.y - mrShark.position.y, 2));

    float sideX = (mrShark.objective.x - mrShark.position.x) / module;
    float sideY = (mrShark.objective.y - mrShark.position.y) / module;

    mrShark.speed = (Vector2){ sideX, sideY };
    printf("DEBUG: Resetting shark\n");
}

void SetFish() {
    int t = 0; // creature type
    for (int i = 0; i < 27; i++){ // generate 27 sea creatures
        SeaCreature sc;
        sc.position = (Vector2) {0,0};
        sc.origin = (Vector2) {0,0};
        if (t > 8) t = 8;
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
    sharkDirection = 1;
    SetFish();
    //printf("DEBUG: SETTING shark coords x:%f y:%f sx:%f sy:%f\n", mrShark.position.x, mrShark.position.y, mrShark.speed.x, mrShark.speed.y);
    //printf("DEBUG: SETTING player coords x:%f y:%f\n", playerPosition.x, playerPosition.y);
}

void HurtShark() {
    if (!sharkBitten || SharkHealth <= 0) return;
    printf("DEBUG: Shark was bitten. Timer: %i\n", SharkHurtTimer);
    mrShark.position = (Vector2){ mrShark.position.x, mrShark.position.y };
    mrShark.objective = (Vector2){ mrShark.position.x, mrShark.position.y }; // pause for a sec
    if (SharkHurtTimer >= 60) {
        SharkHealth--;
        sharkDirection = sharkDirection * -1; // change direction
        SetShark(); // reset exactly where he's headed
        sharkBitten = false;
        SharkHurtTimer = 0;
        return;
    }
    SharkHurtTimer++;
}

void SharkRoam() {
    if (mrShark.active)
    {
        mrShark.objective = playerPosition;

        if (SharkHealth <= 0 && mrShark.position.y >= (float)screenHeight) {
            printf("************* SHARK DIED **************\n");
            mrShark.position.x = -100;
            mrShark.position.y = -100;
            mrShark.active = false;
            score = score + 100;
            return;
        }

        HurtShark();
        if (sharkBitten) return;
        if (SharkHealth > 0) {
            mrShark.position.x += mrShark.speed.x;
            mrShark.position.y += mrShark.speed.y;
        } else {
            mrShark.position.y++;
            return;
        }

        if (mrShark.position.x <= -20 || mrShark.position.x >= (float)screenWidth - 20 || mrShark.position.y <= -20 || mrShark.position.y >= (float)screenHeight - 20) {
            SetShark();
            if (mrShark.position.x <= 5)
                sharkDirection = -1;
            else
                sharkDirection = 1;
            if (sharkBounces == sharkMaxBounces) {
                mrShark.position.x = -100;
                mrShark.position.y = -100;
                mrShark.active = false;
            }
            sharkBounces++;
        }
    } else {
        if (SharkSpawnTimer >= 900) { // 15 seconds
            SharkHealth = 10;
            sharkDirection = 1;
            mrShark.active = true;
            mrShark.position = (Vector2){ (float)screenWidth - 20, 20 };
            SharkSpawnTimer = 0;
        }
        printf("DEBUG: SharkSpawnTimer: %i\n", SharkSpawnTimer);
        SharkSpawnTimer++;
    }
}

void FishSpawn() {
    if (FishSpawnTimer >= 120){
        int pickCreature = 0;
        pickCreature = GetRandomNum(0,26);
        if (!creatures[pickCreature].active){
            srand ( time(NULL) );
            if (creatures[pickCreature].type == 8 && playerRank < 3) return; // no need to spawn jellyfish early in the game
            if (creatures[pickCreature].type == 7 && playerRank < 2) return; // seahorses can wait a bit
            creatures[pickCreature].jump = false;
            creatures[pickCreature].active = true;
            int pickSide[2] = {20,(float)screenWidth - 20};
            float pickHeight = GetRandomNum(20,screenHeight - 50);
            float ps = pickSide[rand()%2];
            if (creatures[pickCreature].type == 5 || creatures[pickCreature].type == 6) pickHeight = (float)screenHeight - 40;
            creatures[pickCreature].origin = (Vector2){ps,pickHeight};
            creatures[pickCreature].position = (Vector2){ps,pickHeight};
            printf("DEBUG: Spawning Fish coords x:%f y:%f type:%i active:%i\n", ps, pickHeight, creatures[pickCreature].type, creatures[pickCreature].active);
        }
        FishSpawnTimer = 0;
    }
    FishSpawnTimer++;
}

void CrustJump(int CreatureID) {
    if (creatures[CreatureID].type != 5 && creatures[CreatureID].type != 6) return; // only crustaceans
    if (creatures[CreatureID].position.y > screenHeight - 128 && creatures[CreatureID].jump) // go up
        creatures[CreatureID].position.y -= 2.0f;
    else if (creatures[CreatureID].position.y <= screenHeight - 128 && creatures[CreatureID].jump) // stop going up
        creatures[CreatureID].jump = false;
    else if (creatures[CreatureID].position.y < screenHeight - 40 && !creatures[CreatureID].jump) // come back down
        creatures[CreatureID].position.y += 2.0f;
}

void FishMoveAndDeSpawn() {
    for (int i = 0; i < 27; i++){
        if (creatures[i].active) {
            // move
            if (creatures[i].origin.x == 20)
                creatures[i].position.x = creatures[i].position.x + creatureSpeed[creatures[i].type];
            else if (creatures[i].origin.x == (float)screenWidth - 20)
                creatures[i].position.x = creatures[i].position.x - creatureSpeed[creatures[i].type];
            // crustacean jump
            if (creatures[i].type == 5 || creatures[i].type == 6){
                if (GetRandomNum(0,500) == 90) {
                    creatures[i].jump = true;
                    //printf("DEBUG: JUMP\n");
                }
                CrustJump(i);
            }
            // de-spawn
            if ((creatures[i].origin.x == ((float)screenWidth - 20) && creatures[i].position.x < 0) ||
                (creatures[i].origin.x == 20 && creatures[i].position.x > (float)screenWidth )) {
                    creatures[i].position.x = -10;
                    creatures[i].position.y = -10;
                    creatures[i].active = false;
                    printf("DEBUG: DE-Spawning Fish active:%i\n", creatures[i].active);
                }
        }
    }
}

void PlayerBit() {
    lives--;
    playerPosition = (Vector2){ -200,-200 };
    playerDead = true;
    if (lives < 0) {
        GameOver = true;
    }
}

int main(void)
{
    // Initialization
    InitWindow(screenWidth, screenHeight, "Shark! Shark!");
    SetVars();
    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

    Texture2D FishTexturesRight[5] = { LoadTexture("./resources/rank1.png"), LoadTexture("resources/rank2.png"), LoadTexture("resources/rank3.png"), LoadTexture("resources/rank4.png"), LoadTexture("resources/rank5.png") };
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
    Texture2D SeaHorseTextureLeft = LoadTexture("resources/seahorse.png");
    Image SeaHorseImageRight = LoadImage("resources/seahorse.png");
    ImageFlipHorizontal(&SeaHorseImageRight);
    Texture2D SeaHorseTextureRight = LoadTextureFromImage(SeaHorseImageRight);
    Texture2D CrabTexture = LoadTexture("resources/crab.png");
    Texture2D LobsterTextureRight = LoadTexture("resources/lobster.png");
    Image LobsterImageLeft = LoadImage("resources/lobster.png");
    ImageFlipHorizontal(&LobsterImageLeft);
    Texture2D LobsterTextureLeft = LoadTextureFromImage(LobsterImageLeft);
    Texture2D JellyfishTexture = LoadTexture("resources/jellyfish.png");

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        struct Rectangle playerRec = { playerPosition.x, playerPosition.y, 16, 16 };

        // these should flip depending on which direction shark is facing
        struct Rectangle sharkTailRec = { mrShark.position.x + (SharkImage.width * 4 / 2), mrShark.position.y, SharkImage.width * 4 / 2, SharkImage.height * 4 };
        struct Rectangle sharkBiteRec = { mrShark.position.x, mrShark.position.y, SharkImage.width * 4 / 2, SharkImage.height * 4 };
        if (sharkDirection == -1){
            sharkTailRec = (Rectangle){ mrShark.position.x, mrShark.position.y, SharkImage.width * 4 / 2, SharkImage.height * 4 };
            sharkBiteRec = (Rectangle){ mrShark.position.x + (SharkImage.width * 4 / 2), mrShark.position.y, SharkImage.width * 4 / 2, SharkImage.height * 4 };
        }

        if (IsKeyPressed(KEY_P)) { if (pause) pause = false; else pause = true; }
        if (IsKeyPressed(KEY_ENTER) && GameOver) { SetVars(); printf("restarting game"); }
        if (IsKeyPressed(KEY_ENTER) && playerDead && !pause && !GameOver) { playerDead = false; playerPosition.x = (float)screenWidth / 2; playerPosition.y = (float)screenHeight / 2; }

        if (!pause && !GameOver) {
            if (IsKeyDown(KEY_RIGHT) && playerPosition.x < screenWidth && !playerDead) { playerPosition.x += 2.0f; playerDirection = -1; }
            if (IsKeyDown(KEY_LEFT) && playerPosition.x > 0 && !playerDead) { playerPosition.x -= 2.0f; playerDirection = 1; }
            if (IsKeyDown(KEY_UP) && playerPosition.y > 0 && !playerDead) playerPosition.y -= 2.0f;
            if (IsKeyDown(KEY_DOWN) && playerPosition.y < screenHeight - 32 && !playerDead) playerPosition.y += 2.0f;
            
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
            if (playerDead)
                DrawText("PLAYER DIED\n\nPRESS ENTER TO SPAWN", screenWidth / 2 - 50, screenHeight / 2 - 50, 20, WHITE);
            
            if (CheckCollisionRecs(playerRec, sharkBiteRec) && SharkHealth > 0) { // shark bit player
                PlayerBit();
            } else if (CheckCollisionRecs(playerRec, sharkTailRec) && SharkHealth > 0) { // player bit shark on tail
                sharkBitten = true;
            }

            // draw player fish
            Vector2 PlayerGoTo = { playerPosition.x, playerPosition.y };
            if (playerDirection == 1) {
                DrawTextureEx(FishTexturesLeft[playerRank], PlayerGoTo, 0, 2, WHITE);
            } else {
                DrawTextureEx(FishTexturesRight[playerRank], PlayerGoTo, 0, 2, WHITE);
            }

            // draw shark
            if (mrShark.active) {
                Vector2 GoTo = { mrShark.position.x, mrShark.position.y };
                if (SharkHealth > 0) {
                    if (sharkDirection == 1) { // left
                        if (SharkHurtTimer%10 && SharkHurtTimer > 0) DrawTextureEx(SharkTextureLeft, GoTo, 0, 4, RED);
                        else DrawTextureEx(SharkTextureLeft, GoTo, 0, 4, YELLOW);
                    }
                    else { // right
                        if (SharkHurtTimer%10 && SharkHurtTimer > 0) DrawTextureEx(SharkTexture, GoTo, 0, 4, RED);
                        else DrawTextureEx(SharkTexture, GoTo, 0, 4, YELLOW);
                    }
                } else {
                    DrawTextureEx(SharkDeadTexture, GoTo, 0, 4, BLACK);
                }
            }

            // for each fish, check collisions and draw on screen
            for (int i = 0; i < 27; i++){
                if (creatures[i].active){
                    if (creatures[i].type < 0 || creatures[i].type > 8) continue;
                    int height = 16;
                    if (creatures[i].type == 8)
                        height = 64;
                    struct Rectangle FishRec = { creatures[i].position.x, creatures[i].position.y, 16, height };
                    if (CheckCollisionRecs(playerRec, FishRec)) {
                        if ((playerRank + 1) >= creatureRank[creatures[i].type]) {
                            creatures[i].position.y = -10;
                            creatures[i].position.x = -10;
                            creatures[i].active = false;
                            score = score + 100;
                            if (score % 1000 == 0 && playerRank < 4 ){
                                playerRank++;
                                //printf("************** PLAYER RANK IS NOW %i ***************\n", playerRank);
                            }
                            else if (score % 1000 == 0 && playerRank >= 4) {
                                lives++;
                            }
                        } else {
                            //printf("**************** BITTEN BY A FISH. X:%f Y:%f ACTIVE:%i TYPE:%i ********************\n", creatures[i].position.x, creatures[i].position.y, creatures[i].active, creatures[i].type);
                            PlayerBit();
                        }
                    }

                    if (creatures[i].type == 5)
                        DrawTextureEx(CrabTexture, (Vector2) { creatures[i].position.x, creatures[i].position.y }, 0, 2.5, YELLOW);
                    else if (creatures[i].type == 6) {
                        if (creatures[i].origin.x <= 20)
                            DrawTextureEx(LobsterTextureLeft, (Vector2) { creatures[i].position.x, creatures[i].position.y }, 0, 2.3, YELLOW);
                        else
                            DrawTextureEx(LobsterTextureRight, (Vector2) { creatures[i].position.x, creatures[i].position.y }, 0, 2.3, YELLOW);
                    } else if (creatures[i].type <= 4 && creatures[i].type >= 0) {
                        Vector2 GoTo = { creatures[i].position.x, creatures[i].position.y };
                        if (creatures[i].origin.x <= 20)
                            DrawTextureEx(FishTexturesRight[creatures[i].type], GoTo, 0, 2, YELLOW); // RIGHT
                        else
                            DrawTextureEx(FishTexturesLeft[creatures[i].type], GoTo, 0, 2, YELLOW); // LEFT
                    } else if (creatures[i].type == 7) {
                        if (creatures[i].origin.x <= 20)
                            DrawTextureEx(SeaHorseTextureLeft, (Vector2) { creatures[i].position.x, creatures[i].position.y }, 0, 2.2, YELLOW);
                        else
                            DrawTextureEx(SeaHorseTextureRight, (Vector2) { creatures[i].position.x, creatures[i].position.y }, 0, 2.2, YELLOW);
                    }
                    else if (creatures[i].type == 8)
                        DrawTextureEx(JellyfishTexture, (Vector2) { creatures[i].position.x, creatures[i].position.y }, 0, 4, YELLOW);
                }
            }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}

