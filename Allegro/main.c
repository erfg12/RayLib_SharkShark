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

BITMAP *screen2; //Double buffer

BITMAP *shark;
BITMAP *lobster;
BITMAP *crab;
BITMAP *shark_dead;
BITMAP *seahorse;
BITMAP *jellyfish;

BITMAP* fish[5];

const int screenWidth = 320;
const int screenHeight = 200;

typedef struct Rectangle {
    float x;
    float y;
    float w;
    float h;
} Rectangle;

 volatile int ticks = 0;

void deinit() {
    clear_keybuf();
}

 void ticker(){
   ticks++;
 }

void abort_on_error(const char *message){
	 if (screen != NULL){
	    set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
	 }
	 allegro_message("%s.\n %s\n", message, allegro_error);
	 exit(-1);
}

int CheckCollisionRecs(Rectangle r1, Rectangle r2) {
  if (r1.x + r1.w >= r2.x &&    // r1 right edge past r2 left
      r1.x <= r2.x + r2.w &&    // r1 left edge past r2 right
      r1.y + r1.h >= r2.y &&    // r1 top edge past r2 bottom
      r1.y <= r2.y + r2.h) {    // r1 bottom edge past r2 top
        return 1;
  }
  return 0;
}

void init() {
    int depth, res;
    allegro_init();

   /* SETUP THE MAIN TIMER TO 60 FPS */
   install_timer(); //Setup the timer
   LOCK_VARIABLE(ticks); //Set timer variable
   //LOCK_FUNCTION(ticker); //Set timer function
   ticker();
   install_int_ex(ticker, BPS_TO_TIMER(60));

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

int main(void)
{
    init();
    SetVars(screenWidth, screenHeight);

    shark = load_bitmap("assets/shark.bmp", NULL);
    shark_dead = load_bitmap("assets/shark_dead.bmp", NULL);
    seahorse = load_bitmap("assets/seahorse.bmp", NULL);
    lobster = load_bitmap("assets/lobster.bmp", NULL);
    crab = load_bitmap("assets/crab.bmp", NULL);
    fish[0] = load_bitmap("assets/rank1.bmp", NULL);
    fish[1] = load_bitmap("assets/rank2.bmp", NULL);
    fish[2] = load_bitmap("assets/rank3.bmp", NULL);
    fish[3] = load_bitmap("assets/rank4.bmp", NULL);
    fish[4] = load_bitmap("assets/rank5.bmp", NULL);

    while (!key[KEY_ESC]) {
        while(ticks == 0) {
            rest(1); //Wait for a full tick
        }

        while(ticks > 0) { // limit game to 60 fps
         int old_ticks = ticks;

        clear_bitmap(screen2);
        rectfill(screen2,0,0,screen->w,screen->h, makecol(0,0,255));
        
        struct Rectangle playerRec = { playerPosition.x, playerPosition.y, 16, 16 };

        // these should flip depending on which direction shark is facing
        struct Rectangle sharkTailRec = { mrShark.position.x + (32 * 4 / 2), mrShark.position.y, 32 * 4 / 2, 16 * 4 };
        struct Rectangle sharkBiteRec = { mrShark.position.x, mrShark.position.y, 32 * 4 / 2, 16 * 4 };
        if (sharkDirection == -1){
            sharkTailRec = (Rectangle){ mrShark.position.x, mrShark.position.y, 32 * 4 / 2, 16 * 4 };
            sharkBiteRec = (Rectangle){ mrShark.position.x + (32 * 4 / 2), mrShark.position.y, 32 * 4 / 2, 16 * 4 };
        }

        if (key[KEY_P]) { if (PausedGame == 1) PausedGame = 0; else PausedGame = 1; }
        if ((key[KEY_ENTER]) && GameOver == 1) { SetVars(screenWidth, screenHeight); printf("restarting game"); }
        if (key[KEY_ENTER] && playerDead == 1 && PausedGame == 0 && GameOver == 0) { playerDead = 0; playerPosition.x = (float)screenWidth / 2; playerPosition.y = (float)screenHeight / 2; }

        if (PausedGame == 0 && GameOver == 0) {
            if (key[KEY_RIGHT] && playerPosition.x < screenWidth && playerDead == 0) { playerPosition.x += 2.0f; playerDirection = -1; }
            if (key[KEY_LEFT] && playerPosition.x > 0 && playerDead == 0) { playerPosition.x -= 2.0f; playerDirection = 1; }
            if (key[KEY_UP] && playerPosition.y > 0 && playerDead == 0) playerPosition.y -= 2.0f;
            if (key[KEY_DOWN] && playerPosition.y < screenHeight - 32 && playerDead == 0) playerPosition.y += 2.0f;
            
            SharkRoam(screenWidth, screenHeight);
            FishSpawn(screenWidth, screenHeight);
            FishMoveAndDeSpawn(screenWidth, screenHeight);
        }

        // DRAWING
        textprintf_ex(screen2, font, 10, 10, makecol(255,255,255), -1, "SCORE %4i", score);
        textprintf_ex(screen2, font, (float)screenWidth-100, 10, makecol(255,255,255), -1, "%4i LIVES", lives);
            if (GameOver == 1)
                textprintf_ex(screen2, font, 10, screenHeight / 2 - 50, makecol(255,255,255), -1, "GAME OVER! YOUR SCORE: %4i - PRESS ENTER TO RESTART GAME", score);
            if (PausedGame == 1)
                textprintf_ex(screen2, font, 10, screenHeight / 2 - 50, makecol(255,255,255), -1, "PAUSED - PRESS P TO RESUME");
            if (playerDead == 1)
                textprintf_ex(screen2, font, 10, screenHeight / 2 - 50, makecol(255,255,255), -1, "PLAYER DIED, PRESS ENTER TO SPAWN");
            
            if (CheckCollisionRecs(playerRec, sharkBiteRec) == 1 && SharkHealth > 0) { // shark bit player
                PlayerBit();
            } else if (CheckCollisionRecs(playerRec, sharkTailRec) == 1 && SharkHealth > 0) { // player bit shark on tail
                sharkBitten = 1;
            }

            // draw player fish
            Vec2 PlayerGoTo = { playerPosition.x, playerPosition.y };
            if (playerDirection == 1) { // left
                masked_blit(fish[playerRank], screen2, 0, 0, playerPosition.x, playerPosition.y, 16, 16);
            } else { // right
                masked_blit(fish[playerRank], screen2, 16, 0, playerPosition.x, playerPosition.y, 16, 16);
            }
        
            // draw shark
            if (mrShark.active) {
                Vec2 GoTo = { mrShark.position.x, mrShark.position.y };
                if (SharkHealth > 0) {
                    if (sharkDirection == 1) { // left
                        if (SharkHurtTimer%10 && SharkHurtTimer > 0) masked_blit(shark, screen2, 32, 0, GoTo.x, GoTo.y, 32, 16);
                        else masked_blit(shark, screen2, 32, 0, GoTo.x, GoTo.y, 32, 16);
                    }
                    else { // right
                        if (SharkHurtTimer%10 && SharkHurtTimer > 0) masked_blit(shark, screen2, 0, 0, GoTo.x, GoTo.y, 32, 16);
                        else masked_blit(shark, screen2, 0, 0, GoTo.x, GoTo.y, 32, 16);
                    }
                } else {
                    draw_sprite(screen2, shark_dead, GoTo.x, GoTo.y);
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
                            creatures[i].active = 0;
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

                    if (creatures[i].type == 5) {
                        masked_blit(crab, screen2, 0, 0, creatures[i].position.x, creatures[i].position.y, 16, 16);
                    } else if (creatures[i].type == 6) {
                        if (creatures[i].origin.x <= 20) // left
                            masked_blit(lobster, screen2, 48, 0, creatures[i].position.x, creatures[i].position.y, 16, 16);
                        else // right
                            masked_blit(lobster, screen2, 0, 0, creatures[i].position.x, creatures[i].position.y, 16, 16);
                    } else if (creatures[i].type <= 4 && creatures[i].type >= 0) {
                        Vec2 GoTo = { creatures[i].position.x, creatures[i].position.y };
                        if (creatures[i].origin.x <= 20) // left
                            masked_blit(fish[creatures[i].type], screen2, 0, 0, creatures[i].position.x, creatures[i].position.y, 16, 16);
                        else // right
                            masked_blit(fish[creatures[i].type], screen2, 16, 0, creatures[i].position.x, creatures[i].position.y, 16, 16);
                    } else if (creatures[i].type == 7) {
                        if (creatures[i].origin.x <= 20)
                            masked_blit(seahorse, screen2, 48, 0, creatures[i].position.x, creatures[i].position.y, 16, 16);
                        else
                            masked_blit(seahorse, screen2, 0, 0, creatures[i].position.x, creatures[i].position.y, 16, 16);
                    }
                    else if (creatures[i].type == 8)
                        masked_blit(jellyfish, screen2, 0, 0, creatures[i].position.x, creatures[i].position.y, 16, 16);
                }
            }
            blit(screen2, screen, 0, 0, 0, 0, screen->w, screen->h);
            ticks--;
            if(old_ticks <= ticks) break; //Time up! Beggin drawing
        }
            //release_screen();
    }

    deinit();
    return 0;
}