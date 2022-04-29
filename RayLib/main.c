#include "raylib.h"
#include "rlgl.h"

#include "../shared.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

const int screenWidth = 800;
const int screenHeight = 450;
Vector2 StoredMousePos;

int main(void)
{
    // Initialization
    InitWindow(screenWidth, screenHeight, "Shark! Shark!");
    SetVars(screenWidth, screenHeight);
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

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !playerDead && !PausedGame && !GameOver) { LeftClick = true; StoredMousePos = GetMousePosition(); }
        if (IsKeyPressed(KEY_P)) { if (PausedGame) PausedGame = false; else PausedGame = true; }
        if ((IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) && GameOver) { SetVars(screenWidth, screenHeight); printf("restarting game"); }
        if (IsKeyPressed(KEY_ENTER) && playerDead && !PausedGame && !GameOver) { playerDead = false; playerPosition.x = (float)screenWidth / 2; playerPosition.y = (float)screenHeight / 2; }

        if (!PausedGame && !GameOver) {
            if (LeftClick && !playerDead) { // click to move
                if (playerPosition.x < StoredMousePos.x) {
                    playerPosition.x += 2.0f;
                    playerDirection = -1;
                }
                else {
                    playerPosition.x -= 2.0f;
                    playerDirection = 1;
                }
                if (playerPosition.y < StoredMousePos.y)
                    playerPosition.y += 2.0f;
                else
                    playerPosition.y -= 2.0f;
                if (playerPosition.x >= StoredMousePos.x - 2 && playerPosition.x <= StoredMousePos.x + 2 && playerPosition.y >= StoredMousePos.y - 2 && playerPosition.y <= StoredMousePos.y + 2)
                    LeftClick = false; 
            }
            if (IsKeyDown(KEY_RIGHT) && playerPosition.x < screenWidth && !playerDead) { playerPosition.x += 2.0f; playerDirection = -1; }
            if (IsKeyDown(KEY_LEFT) && playerPosition.x > 0 && !playerDead) { playerPosition.x -= 2.0f; playerDirection = 1; }
            if (IsKeyDown(KEY_UP) && playerPosition.y > 0 && !playerDead) playerPosition.y -= 2.0f;
            if (IsKeyDown(KEY_DOWN) && playerPosition.y < screenHeight - 32 && !playerDead) playerPosition.y += 2.0f;
            
            SharkRoam(screenWidth, screenHeight);
            FishSpawn(screenWidth, screenHeight);
            FishMoveAndDeSpawn(screenWidth, screenHeight);
        }

        // DRAW
        BeginDrawing();
            ClearBackground(BLUE);
            DrawText(TextFormat("SCORE %4i", score), 10, 10, 20, RAYWHITE);
            DrawText(TextFormat("%4i LIVES", lives), (float)screenWidth - 120, 10, 20, RAYWHITE);
            if (GameOver) 
                DrawText(TextFormat("GAME OVER!\n\nYOUR SCORE: %4i\n\nPRESS ENTER TO RESTART GAME", score), screenWidth / 2 - 100, screenHeight / 2 - 100, 20, RED);
            if (PausedGame)
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

