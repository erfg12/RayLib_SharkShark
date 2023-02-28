#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include "..\..\shared.h"

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

SDL_Texture* shark;
SDL_Texture* lobster;
SDL_Surface* lobster_surface;
SDL_Texture* crab;
SDL_Texture* shark_dead;
SDL_Texture* seahorse;
SDL_Texture* jellyfish;

SDL_Texture* fish[5];
SDL_Surface* fish_surfaces[5];

SDL_Color color_white = { 255,255,255,255 };
SDL_Color color_black = { 0,0,0,255 };

int playerSpeed = 4;

typedef struct Rectangle {
	float x;
	float y;
	float w;
	float h;
} Rectangle;

int CheckCollisionRecs(Rectangle r1, Rectangle r2) {
	if (r1.x + r1.w >= r2.x &&    // r1 right edge past r2 left
		r1.x <= r2.x + r2.w &&    // r1 left edge past r2 right
		r1.y + r1.h >= r2.y &&    // r1 top edge past r2 bottom
		r1.y <= r2.y + r2.h) {    // r1 bottom edge past r2 top
		return 1;
	}
	return 0;
}

int main(int argc, char* args[])
{
	SDL_Window* window = SDL_CreateWindow("SharkShark", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN); // The window we'll be rendering to
	SDL_Renderer* gRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC); // The window renderer

	SDL_Surface* shark_surface = SDL_LoadBMP("assets/shark.bmp");
	SDL_SetColorKey(shark_surface, SDL_TRUE, SDL_MapRGB(shark_surface->format, 0xFF, 0x0, 0xFF));
	shark = SDL_CreateTextureFromSurface(gRenderer, shark_surface);
	SDL_Surface* shark_dead_surface = SDL_LoadBMP("assets/shark_dead.bmp");
	SDL_SetColorKey(shark_dead_surface, SDL_TRUE, SDL_MapRGB(shark_dead_surface->format, 0xFF, 0x0, 0xFF));
	shark_dead = SDL_CreateTextureFromSurface(gRenderer, shark_dead_surface);
	SDL_Surface* seahorse_surface = SDL_LoadBMP("assets/seahorse.bmp");
	SDL_SetColorKey(seahorse_surface, SDL_TRUE, SDL_MapRGB(seahorse_surface->format, 0xFF, 0x0, 0xFF));
	seahorse = SDL_CreateTextureFromSurface(gRenderer, seahorse_surface);
	SDL_Surface* lobster_surface = SDL_LoadBMP("assets/lobster.bmp");
	SDL_SetColorKey(lobster_surface, SDL_TRUE, SDL_MapRGB(lobster_surface->format, 0xFF, 0x0, 0xFF));
	lobster = SDL_CreateTextureFromSurface(gRenderer, lobster_surface);
	SDL_Surface* crab_surface = SDL_LoadBMP("assets/crab.bmp");
	SDL_SetColorKey(crab_surface, SDL_TRUE, SDL_MapRGB(crab_surface->format, 0xFF, 0x0, 0xFF));
	crab = SDL_CreateTextureFromSurface(gRenderer, crab_surface);
	fish_surfaces[0] = SDL_LoadBMP("assets/rank1.bmp");
	SDL_SetColorKey(fish_surfaces[0], SDL_TRUE, SDL_MapRGB(fish_surfaces[0]->format, 0xFF, 0x0, 0xFF));
	fish[0] = SDL_CreateTextureFromSurface(gRenderer, fish_surfaces[0]);
	fish_surfaces[1] = SDL_LoadBMP("assets/rank2.bmp");
	SDL_SetColorKey(fish_surfaces[1], SDL_TRUE, SDL_MapRGB(fish_surfaces[1]->format, 0xFF, 0x0, 0xFF));
	fish[1] = SDL_CreateTextureFromSurface(gRenderer, fish_surfaces[1]);
	fish_surfaces[2] = SDL_LoadBMP("assets/rank3.bmp");
	SDL_SetColorKey(fish_surfaces[2], SDL_TRUE, SDL_MapRGB(fish_surfaces[2]->format, 0xFF, 0x0, 0xFF));
	fish[2] = SDL_CreateTextureFromSurface(gRenderer, fish_surfaces[2]);
	fish_surfaces[3] = SDL_LoadBMP("assets/rank4.bmp");
	SDL_SetColorKey(fish_surfaces[3], SDL_TRUE, SDL_MapRGB(fish_surfaces[3]->format, 0xFF, 0x0, 0xFF));
	fish[3] = SDL_CreateTextureFromSurface(gRenderer, fish_surfaces[3]);
	fish_surfaces[4] = SDL_LoadBMP("assets/rank5.bmp");
	SDL_SetColorKey(fish_surfaces[4], SDL_TRUE, SDL_MapRGB(fish_surfaces[4]->format, 0xFF, 0x0, 0xFF));
	fish[4] = SDL_CreateTextureFromSurface(gRenderer, fish_surfaces[4]);

	SDL_FreeSurface(fish_surfaces[0]);
	SDL_FreeSurface(fish_surfaces[1]);
	SDL_FreeSurface(fish_surfaces[2]);
	SDL_FreeSurface(fish_surfaces[3]);
	SDL_FreeSurface(fish_surfaces[4]);
	SDL_FreeSurface(shark_surface);
	SDL_FreeSurface(shark_dead_surface);
	SDL_FreeSurface(seahorse_surface);
	//SDL_FreeSurface(lobster_surface);
	SDL_FreeSurface(crab_surface);

	const Uint8* keys = SDL_GetKeyboardState(NULL);

	if (!shark)
		printf("Error creating texture: %s\n", SDL_GetError());

	if (TTF_Init() < 0) {
		printf("SDL TTF could not initialize! %s", SDL_GetError());
	}

	TTF_Font* font = TTF_OpenFont("assets/pixantiqua.ttf", 25);

	if (SDL_Init(SDL_INIT_VIDEO) < 0) // Initialize SDL
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
	}
	else
	{
		if (window == NULL)
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		else
		{
			SDL_Event e;
			int quit = 0;
			SetVars(SCREEN_WIDTH, SCREEN_HEIGHT);
			while (quit == 0) {
				Uint32 ticks = SDL_GetTicks();

				SDL_SetRenderDrawColor(gRenderer, 0, 0, 255, 255); // blue like water
				SDL_RenderClear(gRenderer);

				struct Rectangle playerRec = { playerPosition.x, playerPosition.y, 16, 16 };

				// these should flip depending on which direction shark is facing
				struct Rectangle sharkTailRec = { mrShark.position.x + (64 / 2), mrShark.position.y, 64 / 2, 32 };
				struct Rectangle sharkBiteRec = { mrShark.position.x, mrShark.position.y, 64 / 2, 32 };
				if (sharkDirection == -1) {
					sharkTailRec = (Rectangle){ mrShark.position.x, mrShark.position.y, 64 / 2, 32 };
					sharkBiteRec = (Rectangle){ mrShark.position.x + (64 / 2), mrShark.position.y, 64 / 2, 32 };
				}

				// NPCs move
				if (PausedGame == 0 && GameOver == 0 && mainMenu == 0) {
					SharkRoam(SCREEN_WIDTH, SCREEN_HEIGHT);
					FishSpawn(SCREEN_WIDTH, SCREEN_HEIGHT);
					FishMoveAndDeSpawn(SCREEN_WIDTH, SCREEN_HEIGHT);
				}

				// check for button presses
				while (SDL_PollEvent(&e)) {
					if (e.type == SDL_KEYDOWN)
					{
						if (keys[SDL_SCANCODE_P]) { if (PausedGame == 1) PausedGame = 0; else PausedGame = 1; }
						if ((keys[SDL_SCANCODE_RETURN]) && GameOver == 1) { SetVars(SCREEN_WIDTH, SCREEN_HEIGHT); printf("restarting game"); }
						if (keys[SDL_SCANCODE_RETURN] && playerDead == 1 && PausedGame == 0 && GameOver == 0) { playerDead = 0; playerPosition.x = (float)SCREEN_WIDTH / 2; playerPosition.y = (float)SCREEN_HEIGHT / 2; }
					}

					if (mainMenu == 1) {
						if (keys[SDL_SCANCODE_S]) { mainMenu = 0; }
					}

					if (PausedGame == 0 && GameOver == 0) {
						if (e.type == SDL_KEYDOWN)
						{
							if (keys[SDL_SCANCODE_RIGHT] && playerPosition.x < SCREEN_WIDTH && playerDead == 0) { playerPosition.x += playerSpeed; playerDirection = -1; }
							if (keys[SDL_SCANCODE_LEFT] && playerPosition.x > 0 && playerDead == 0) { playerPosition.x -= playerSpeed; playerDirection = 1; }
							if (keys[SDL_SCANCODE_UP] && playerPosition.y > 0 && playerDead == 0) playerPosition.y -= playerSpeed;
							if (keys[SDL_SCANCODE_DOWN] && playerPosition.y < SCREEN_HEIGHT - 15 && playerDead == 0) playerPosition.y += playerSpeed;
						}
					}

					if (e.type == SDL_QUIT)
						quit = 1;
				}

				char UI_Score_t[255];
				sprintf_s(UI_Score_t, 255, "SCORE %d", score);
				SDL_Surface* UI_Score = TTF_RenderText_Solid(font, UI_Score_t, color_white);
				SDL_Texture* UI_Score_text = SDL_CreateTextureFromSurface(gRenderer, UI_Score);
				SDL_Rect UI_Score_renderQuad = { 10, 10, UI_Score->w, UI_Score->h };
				SDL_RenderCopy(gRenderer, UI_Score_text, NULL, &UI_Score_renderQuad);
				char UI_Lives_t[255];
				sprintf_s(UI_Lives_t, 255, "%i LIVES", lives);
				SDL_Surface* UI_Lives = TTF_RenderText_Solid(font, UI_Lives_t, color_white);
				SDL_Texture* UI_Lives_text = SDL_CreateTextureFromSurface(gRenderer, UI_Lives);
				SDL_Rect UI_Lives_renderQuad = { SCREEN_WIDTH - 120, 10, UI_Lives->w, UI_Lives->h };
				SDL_RenderCopy(gRenderer, UI_Lives_text, NULL, &UI_Lives_renderQuad);
				if (GameOver) {
					char UI_gameover_t[255];
					sprintf_s(UI_gameover_t, 255, "GAME OVER!\n\nYOUR SCORE: %4i\n\nPRESS ENTER TO RESTART GAME", score);
					SDL_Surface* UI_gameover = TTF_RenderText_Blended_Wrapped(font, UI_gameover_t, color_white, 800);
					SDL_Texture* UI_gameover_text = SDL_CreateTextureFromSurface(gRenderer, UI_gameover);
					SDL_Rect UI_gameover_renderQuad = { SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 100, UI_gameover->w, UI_gameover->h };
					SDL_RenderCopy(gRenderer, UI_gameover_text, NULL, &UI_gameover_renderQuad);
				}
				if (PausedGame) {
					SDL_Surface* UI_pause = TTF_RenderText_Blended_Wrapped(font, "PAUSED\n\nPRESS P TO RESUME", color_white, 800);
					SDL_Texture* UI_pause_text = SDL_CreateTextureFromSurface(gRenderer, UI_pause);
					SDL_Rect UI_pause_renderQuad = { SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2 - 50, UI_pause->w, UI_pause->h };
					SDL_RenderCopy(gRenderer, UI_pause_text, NULL, &UI_pause_renderQuad);
				}
				if (playerDead) {
					SDL_Surface* UI_died = TTF_RenderText_Blended_Wrapped(font, "PLAYER DIED\n\nPRESS ENTER TO SPAWN", color_white, 800);
					SDL_Texture* UI_died_text = SDL_CreateTextureFromSurface(gRenderer, UI_died);
					SDL_Rect UI_died_renderQuad = { SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2 - 50, UI_died->w, UI_died->h };
					SDL_RenderCopy(gRenderer, UI_died_text, NULL, &UI_died_renderQuad);
				}
				if (mainMenu == 1) {
					SDL_Surface* UI_mainmenu = TTF_RenderText_Blended_Wrapped(font, "SHARK! SHARK!\nre-created by Jacob Fliss\n\n[S]tart Game\n[Q]uit Game", color_white, 800);
					SDL_Texture* UI_mainmenu_text = SDL_CreateTextureFromSurface(gRenderer, UI_mainmenu);
					SDL_Rect UI_mainmenu_renderQuad = { SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2 - 90, UI_mainmenu->w, UI_mainmenu->h };
					SDL_RenderCopy(gRenderer, UI_mainmenu_text, NULL, &UI_mainmenu_renderQuad);
				}

				if (CheckCollisionRecs(playerRec, sharkBiteRec) && SharkHealth > 0) { // shark bit player
					PlayerBit();
				}
				else if (CheckCollisionRecs(playerRec, sharkTailRec) && SharkHealth > 0) { // player bit shark on tail
					sharkBitten = 1;
				}

				// draw player fish
				SDL_Rect PosSize = { (int)playerPosition.x, (int)playerPosition.y, 16, 16 };
				if (playerDirection == 1) { // left
					SDL_RenderCopyEx(gRenderer, fish[playerRank], NULL, &PosSize, 0, NULL, SDL_FLIP_NONE);
				}
				else { // right
					SDL_RenderCopyEx(gRenderer, fish[playerRank], NULL, &PosSize, 0, NULL, SDL_FLIP_HORIZONTAL);
				}

				// draw shark
				if (mrShark.active) {
					SDL_Rect GoTo = { (int)mrShark.position.x, (int)mrShark.position.y, 64, 32 };

					if (SharkHealth > 0) {
						if (SharkHurtTimer % 10 && SharkHurtTimer > 0)
							SDL_SetTextureColorMod(shark, 255, 0, 0);
						else
							SDL_SetTextureColorMod(shark, 255, 255, 0);
						if (sharkDirection == 1) { // left
							SDL_RenderCopyEx(gRenderer, shark, NULL, &GoTo, 0, NULL, SDL_FLIP_NONE);
						}
						else { // right
							SDL_RenderCopyEx(gRenderer, shark, NULL, &GoTo, 0, NULL, SDL_FLIP_HORIZONTAL);
						}
					}
					else {
						SDL_SetTextureColorMod(shark, 0, 0, 0);
					}
				}

				// for each fish, check collisions and draw on screen
				int animChange = (ticks / 200) % 2;
				for (int i = 0; i < 27; i++) {
					if (creatures[i].active) {
						if (creatures[i].type < 0 || creatures[i].type > 8) continue;
						int height = 16;
						if (creatures[i].type == 8)
							height = 64;
						struct Rectangle FishRec = { creatures[i].position.x, creatures[i].position.y, 16, height };
						if (CheckCollisionRecs(playerRec, FishRec)) {
							if ((playerRank + 1) >= creatureRank[creatures[i].type]) {
								creatures[i].position.y = -10;
								creatures[i].position.x = -10;
								creatures[i].active = 0;
								score = score + 100;
								if (score % 1000 == 0 && playerRank < 4) {
									playerRank++;
									//printf("************** PLAYER RANK IS NOW %i ***************\n", playerRank);
								}
								else if (score % 1000 == 0 && playerRank >= 4) {
									lives++;
								}
							}
							else {
								//printf("**************** BITTEN BY A FISH. X:%f Y:%f ACTIVE:%i TYPE:%i ********************\n", creatures[i].position.x, creatures[i].position.y, creatures[i].active, creatures[i].type);
								PlayerBit();
							}
						}

						SDL_Rect GoTo = { creatures[i].position.x, creatures[i].position.y, 16, 16 };
						SDL_Rect tmp = { animChange * 16, 0, ((animChange + 1) * 16), GoTo.y };
						if (creatures[i].type == 5) {
							SDL_RenderCopyEx(gRenderer, crab, &tmp, &GoTo, 0, NULL, SDL_FLIP_NONE);
						}
						else if (creatures[i].type == 6) {
							
							SDL_Rect tmp = { animChange * 16, 0, ((animChange + 1) * 16), GoTo.y }; // first animation TO-DO: Cycle through animations
							if (creatures[i].origin.x <= 20) {
								SDL_RenderCopyEx(gRenderer, lobster, &tmp, &GoTo, 0, NULL, SDL_FLIP_HORIZONTAL);
							}
							else
								SDL_RenderCopyEx(gRenderer, lobster, &tmp, &GoTo, 0, NULL, SDL_FLIP_NONE);
						}
						else if (creatures[i].type <= 4 && creatures[i].type >= 0) {
							if (creatures[i].origin.x <= 20) {
								SDL_RenderCopyEx(gRenderer, fish[creatures[i].type], NULL, &GoTo, 0, NULL, SDL_FLIP_HORIZONTAL); // RIGHT
							}
							else {
								SDL_RenderCopyEx(gRenderer, fish[creatures[i].type], NULL, &GoTo, 0, NULL, SDL_FLIP_NONE); // LEFT
							}
						}
						else if (creatures[i].type == 7) {
							if (creatures[i].origin.x <= 20)
								SDL_RenderCopyEx(gRenderer, seahorse, &tmp, &GoTo, 0, NULL, SDL_FLIP_NONE);
							else
								SDL_RenderCopyEx(gRenderer, seahorse, &tmp, &GoTo, 0, NULL, SDL_FLIP_HORIZONTAL);
						}
						else if (creatures[i].type == 8)
							SDL_RenderCopyEx(gRenderer, jellyfish, &tmp, &GoTo, 0, NULL, SDL_FLIP_NONE);
					}
				}

				SDL_RenderPresent(gRenderer);
			}
		}
	}
	SDL_DestroyWindow(window); // Destroy window
	SDL_Quit(); // Quit SDL subsystems

	return 0;
}
