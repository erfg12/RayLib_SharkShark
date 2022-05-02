#include <allegro.h>

#include "../shared.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// allegro stuff
#define Y 40
#define X 30
#define P 16

BITMAP *buffer;

BITMAP *shark;
BITMAP *lobster;
BITMAP *crab;
BITMAP *shark_dead;
BITMAP *seahorse;

BITMAP *fish1;
BITMAP *fish2;
BITMAP *fish3;
BITMAP *fish4;
BITMAP *fish5;

const int screenWidth = 320;
const int screenHeight = 200;

struct Rectangle {
    float x,
    float y,
    float w,
    float h
} Rectangle;

int CheckCollisionRecs(Rectangle r1, Rectangle r2) {
  if (r1->x + r1->w >= r2->x &&    // r1 right edge past r2 left
      r1->x <= r2->x + r2->w &&    // r1 left edge past r2 right
      r1->y + r1->h >= r2->y &&    // r1 top edge past r2 bottom
      r1->y <= r2->y + r2->h) {    // r1 bottom edge past r2 top
        return 1;
  }
  return 0;
}

void init() {
    int depth, res;
    allegro_init();
    depth = desktop_color_depth();
    if (depth == 0) depth = 32;
    set_color_depth(depth);
#ifdef _WIN32
    res = set_gfx_mode(GFX_AUTODETECT_WINDOWED, X * P, Y * P, 0, 0);
#else
    res = set_gfx_mode(GFX_AUTODETECT, screenWidth, screenHeight, 0, 0);
#endif
    if (res != 0) {
        allegro_message(allegro_error);
        exit(-1);
    }

    /* INITIALIZE BACK BUFFER (MUST BE AFTER THE GRAPHICS!) */
   screen2 = create_bitmap(screen->w, screen->h); //Set up double buffer
   if(!screen2) {
     allegro_message("Failed to create double buffer. Out of memory?");
     exit(1);
   }

    install_keyboard();
}

void deinit() {
    clear_keybuf();
}

int main(void)
{
    init();

    shark = load_bitmap("resources\\shark.bmp", NULL);
    shark_dead = load_bitmap("resources\\shark_dead.bmp", NULL);
    seahorse = load_bitmap("resources\\seahorse.bmp", NULL);
    lobster = load_bitmap("resources\\lobster.bmp", NULL);
    crab = load_bitmap("resources\\crab.bmp", NULL);
    fish1 = load_bitmap("resources\\rank1.bmp", NULL);
    fish2 = load_bitmap("resources\\rank2.bmp", NULL);
    fish3 = load_bitmap("resources\\rank3.bmp", NULL);
    fish4 = load_bitmap("resources\\rank4.bmp", NULL);
    fish5 = load_bitmap("resources\\rank5.bmp", NULL);

    while (!key[KEY_ESC]) {
        struct Rectangle playerRec = { playerPosition.x, playerPosition.y, 16, 16 };

        // these should flip depending on which direction shark is facing
        struct Rectangle sharkTailRec = { mrShark.position.x + (SharkImage.width * 4 / 2), mrShark.position.y, SharkImage.width * 4 / 2, SharkImage.height * 4 };
        struct Rectangle sharkBiteRec = { mrShark.position.x, mrShark.position.y, SharkImage.width * 4 / 2, SharkImage.height * 4 };
        if (sharkDirection == -1){
            sharkTailRec = (Rectangle){ mrShark.position.x, mrShark.position.y, SharkImage.width * 4 / 2, SharkImage.height * 4 };
            sharkBiteRec = (Rectangle){ mrShark.position.x + (SharkImage.width * 4 / 2), mrShark.position.y, SharkImage.width * 4 / 2, SharkImage.height * 4 };
        }

        if (key[KEY_P]) { if (PausedGame) PausedGame = false; else PausedGame = true; }
        if ((key[KEY_ENTER]) && GameOver) { SetVars(screenWidth, screenHeight); printf("restarting game"); }
        if (key[KEY_ENTER] && playerDead && !PausedGame && !GameOver) { playerDead = false; playerPosition.x = (float)screenWidth / 2; playerPosition.y = (float)screenHeight / 2; }

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
            if (key[KEY_RIGHT] && playerPosition.x < screenWidth && !playerDead) { playerPosition.x += 2.0f; playerDirection = -1; }
            if (key[KEY_LEFT] && playerPosition.x > 0 && !playerDead) { playerPosition.x -= 2.0f; playerDirection = 1; }
            if (key[KEY_UP] && playerPosition.y > 0 && !playerDead) playerPosition.y -= 2.0f;
            if (key[KEY_DOWN] && playerPosition.y < screenHeight - 32 && !playerDead) playerPosition.y += 2.0f;
            
            SharkRoam(screenWidth, screenHeight);
            FishSpawn(screenWidth, screenHeight);
            FishMoveAndDeSpawn(screenWidth, screenHeight);
        }

        // DRAWING
        textprintf_ex(screen2, font, 10, 10, makecol(255,255,255), -1, "SCORE %4i", score);
        textprintf_ex(screen2, font, (float)screenWidth - 25, 10, makecol(255,255,255), -1, "%4i LIVES", lives);
            if (GameOver)
                textprintf_ex(screen2, font, screenWidth / 2 - 50, screenHeight / 2 - 50, makecol(255,255,255), -1, "GAME OVER!\n\nYOUR SCORE: %4i\n\nPRESS ENTER TO RESTART GAME", score);
            if (PausedGame)
                textprintf_ex(screen2, font, screenWidth / 2 - 50, screenHeight / 2 - 50, makecol(255,255,255), -1, "PAUSED\n\nPRESS P TO RESUME");
            if (playerDead)
                textprintf_ex(screen2, font, screenWidth / 2 - 50, screenHeight / 2 - 50, makecol(255,255,255), -1, "PLAYER DIED\n\nPRESS ENTER TO SPAWN");
            
            if (CheckCollisionRecs(playerRec, sharkBiteRec) == 1 && SharkHealth > 0) { // shark bit player
                PlayerBit();
            } else if (CheckCollisionRecs(playerRec, sharkTailRec) == 1 && SharkHealth > 0) { // player bit shark on tail
                sharkBitten = 1;
            }

            // draw player fish // USE draw_sprite(buffer, sprite, x, y);
        /*    Vector2 PlayerGoTo = { playerPosition.x, playerPosition.y };
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
                    if (CheckCollisionRecs(playerRec, FishRec) == 1) {
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
            }*/
    }

    deinit();
    return 0;
}