#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include "..\..\shared.h"

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

int sharkDeathAudioPlayed = 0;

SDL_Surface* shark;
SDL_Surface* shark_dead;
SDL_Surface* lobster;
SDL_Surface* crab;
SDL_Surface* seahorse;
SDL_Surface* jellyfish;

SDL_Surface* fish[5];

SDL_Color color_white = { 255,255,255,255 };
SDL_Color color_black = { 0,0,0,255 };

Mix_Chunk* sharkSpawnSound = NULL;
Mix_Chunk* sharkDeadSound = NULL;
Mix_Chunk* fishBiteSound = NULL;
Mix_Chunk* gameOverSound = NULL;
Mix_Chunk* deadSound = NULL;
Mix_Chunk* fishRankUp = NULL;

Mix_Music* bgMusic = NULL;

int playerSpeed = 2;

//FILE* __cdecl __iob_func(void)
//{
//	FILE _iob[] = { *stdin, *stdout, *stderr };
//	return _iob;
//}

#ifdef main
#undef main
#endif

typedef struct Rectangle {
	float x;
	float y;
	float w;
	float h;
} Rectangle;

#define TICK_INTERVAL    20

static Uint32 next_time;

Uint32 time_left(void)
{
	Uint32 now;

	now = SDL_GetTicks();
	if (next_time <= now)
		return 0;
	else
		return next_time - now;
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

int main(int argc, char* args[])
{
	SDL_Surface* screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_DOUBLEBUF | SDL_HWSURFACE);
	SDL_WM_SetCaption("SharkShark", NULL);
	
	next_time = SDL_GetTicks() + TICK_INTERVAL;

	shark = SDL_LoadBMP("assets/sprites/shark.bmp");
	SDL_SetColorKey(shark, SDL_SRCCOLORKEY, SDL_MapRGB(shark->format, 0xFF, 0x0, 0xFF)); // NOTE: Images must be 24 bit depth to work with color keys
	shark_dead = SDL_LoadBMP("assets/sprites/shark_dead.bmp");
	SDL_SetColorKey(shark_dead, SDL_SRCCOLORKEY, SDL_MapRGB(shark_dead->format, 0xFF, 0x0, 0xFF));
	seahorse = SDL_LoadBMP("assets/sprites/seahorse.bmp");
	SDL_SetColorKey(seahorse, SDL_SRCCOLORKEY, SDL_MapRGB(seahorse->format, 0xFF, 0x0, 0xFF));
	lobster = SDL_LoadBMP("assets/sprites/lobster.bmp");
	SDL_SetColorKey(lobster, SDL_SRCCOLORKEY, SDL_MapRGB(lobster->format, 0xFF, 0x0, 0xFF));
	crab = SDL_LoadBMP("assets/sprites/crab.bmp");
	SDL_SetColorKey(crab, SDL_SRCCOLORKEY, SDL_MapRGB(crab->format, 0xFF, 0x0, 0xFF));
	fish[0] = SDL_LoadBMP("assets/sprites/rank1.bmp");
	SDL_SetColorKey(fish[0], SDL_SRCCOLORKEY, SDL_MapRGB(fish[0]->format, 0xFF, 0x0, 0xFF));
	fish[1] = SDL_LoadBMP("assets/sprites/rank2.bmp");
	SDL_SetColorKey(fish[1], SDL_SRCCOLORKEY, SDL_MapRGB(fish[1]->format, 0xFF, 0x0, 0xFF));
	fish[2] = SDL_LoadBMP("assets/sprites/rank3.bmp");
	SDL_SetColorKey(fish[2], SDL_SRCCOLORKEY, SDL_MapRGB(fish[2]->format, 0xFF, 0x0, 0xFF));
	fish[3] = SDL_LoadBMP("assets/sprites/rank4.bmp");
	SDL_SetColorKey(fish[3], SDL_SRCCOLORKEY, SDL_MapRGB(fish[3]->format, 0xFF, 0x0, 0xFF));
	fish[4] = SDL_LoadBMP("assets/sprites/rank5.bmp");
	SDL_SetColorKey(fish[4], SDL_SRCCOLORKEY, SDL_MapRGB(fish[4]->format, 0xFF, 0x0, 0xFF));

	const Uint8* keys = SDL_GetKeyState(NULL);

	if (!shark)
		printf("Error creating texture: %s\n", SDL_GetError());

	if (TTF_Init() < 0) {
		printf("SDL TTF could not initialize! %s", SDL_GetError());
	}

	TTF_Font* font = TTF_OpenFont("assets/pixantiqua.ttf", 25);

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
	{
		printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
	}

	sharkSpawnSound = Mix_LoadWAV("assets/audio/shark.wav");
	fishBiteSound = Mix_LoadWAV("assets/audio/eat.wav");
	gameOverSound = Mix_LoadWAV("assets/audio/gameover.wav");
	deadSound = Mix_LoadWAV("assets/audio/dead.wav");
	sharkDeadSound = Mix_LoadWAV("assets/audio/shark_dead.wav");
	fishRankUp = Mix_LoadWAV("assets/audio/bigger.wav"); // NOTE: Game can crash on startup if audio bit-rate is too high

	bgMusic = Mix_LoadMUS("assets/audio/bg_music.wav");

	if (SDL_Init(SDL_INIT_EVERYTHING)  == -1) // Initialize SDL
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
	}
	else
	{
		if (screen == NULL)
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		else
		{
			SDL_Event e;
			int quit = 0;
			SetVars(SCREEN_WIDTH, SCREEN_HEIGHT);
			while (quit == 0) {
				Uint32 ticks = SDL_GetTicks();
				SDL_FillRect(screen, NULL, 0x0000FF);// blue like water

				struct Rectangle playerRec = { playerPosition.x, playerPosition.y, 16, 16 };

				if (Mix_PlayingMusic() == 0)
				{
					Mix_PlayMusic(bgMusic, -1);
				}

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
					FishMoveAndDeSpawn(SCREEN_WIDTH, SCREEN_HEIGHT, 16);
				}

				while (SDL_PollEvent(&e)) {
					if (e.type == SDL_QUIT)
						quit = 1;
				}

				// check for button presses
				if (keys[SDLK_p]) { if (PausedGame == 1) PausedGame = 0; else PausedGame = 1; }
				if ((keys[SDLK_RETURN]) && GameOver == 1) { SetVars(SCREEN_WIDTH, SCREEN_HEIGHT); printf("restarting game"); }
				if (keys[SDLK_RETURN] && playerDead == 1 && PausedGame == 0 && GameOver == 0) { playerDead = 0; playerPosition.x = (float)SCREEN_WIDTH / 2; playerPosition.y = (float)SCREEN_HEIGHT / 2; }
				if (mainMenu == 1) {
					if (keys[SDLK_s]) { mainMenu = 0; }
				}
				if (PausedGame == 0 && GameOver == 0 && mainMenu == 0) {
					if ((keys[SDLK_RIGHT] || keys[SDLK_d]) && playerPosition.x < SCREEN_WIDTH && playerDead == 0) { playerPosition.x += playerSpeed; playerDirection = -1; }
					if ((keys[SDLK_LEFT] || keys[SDLK_a]) && playerPosition.x > 0 && playerDead == 0) { playerPosition.x -= playerSpeed; playerDirection = 1; }
					if ((keys[SDLK_UP] || keys[SDLK_w]) && playerPosition.y > 0 && playerDead == 0) playerPosition.y -= playerSpeed;
					if ((keys[SDLK_DOWN] || keys[SDLK_s]) && playerPosition.y < SCREEN_HEIGHT - 15 && playerDead == 0) playerPosition.y += playerSpeed;
				}

				char UI_Score_t[255];
				sprintf_s(UI_Score_t, 255, "SCORE %d", score);
				SDL_Surface* UI_Score = TTF_RenderText_Solid(font, UI_Score_t, color_white);
				SDL_Rect UI_Score_renderQuad = { 10, 10, UI_Score->w, UI_Score->h };
				SDL_BlitSurface(UI_Score, NULL, screen, &UI_Score_renderQuad);
				char UI_Lives_t[255];
				sprintf_s(UI_Lives_t, 255, "%i LIVES", lives);
				SDL_Surface* UI_Lives = TTF_RenderText_Solid(font, UI_Lives_t, color_white);
				SDL_Rect UI_Lives_renderQuad = { SCREEN_WIDTH - 120, 10, UI_Lives->w, UI_Lives->h };
				SDL_BlitSurface(UI_Lives, NULL, screen, &UI_Lives_renderQuad);
				if (GameOver) {
					char UI_gameover_t[255];
					sprintf_s(UI_gameover_t, 255, "GAME OVER!\n\nYOUR SCORE: %4i\n\nPRESS ENTER TO RESTART GAME", score);
					SDL_Surface* UI_gameover = TTF_RenderText_Solid(font, UI_gameover_t, color_white, 800);
					SDL_Rect UI_gameover_renderQuad = { SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 100, UI_gameover->w, UI_gameover->h };
					SDL_BlitSurface(UI_gameover, NULL, screen, &UI_gameover_renderQuad);
				}
				if (PausedGame) {
					SDL_Surface* UI_pause = TTF_RenderText_Solid(font, "PAUSED\n\nPRESS P TO RESUME", color_white, 800);
					SDL_Rect UI_pause_renderQuad = { SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2 - 50, UI_pause->w, UI_pause->h };
					SDL_BlitSurface(UI_pause, NULL, screen, &UI_pause_renderQuad);
				}
				if (playerDead) {
					SDL_Surface* UI_died = TTF_RenderText_Solid(font, "PLAYER DIED\n\nPRESS ENTER TO SPAWN", color_white, 800);
					SDL_Rect UI_died_renderQuad = { SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2 - 50, UI_died->w, UI_died->h };
					SDL_BlitSurface(UI_died, NULL, screen, &UI_died_renderQuad);
				}
				if (mainMenu == 1) {
					SDL_Surface* UI_mainmenu = TTF_RenderText_Blended(font, "SHARK! SHARK!\nre-created by Jacob Fliss\n\n[S]tart Game\n[Q]uit Game", color_white, 800);
					SDL_Rect UI_mainmenu_renderQuad = { SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2 - 90, UI_mainmenu->w, UI_mainmenu->h };
					SDL_BlitSurface(UI_mainmenu, NULL, screen, &UI_mainmenu_renderQuad);
				}

				if (CheckCollisionRecs(playerRec, sharkBiteRec) && SharkHealth > 0) { // shark bit player
					PlayerBit();
					if (lives == 0)
						Mix_PlayChannel(-1, gameOverSound, 0);
					else
						Mix_PlayChannel(-1, deadSound, 0);
				}
				else if (CheckCollisionRecs(playerRec, sharkTailRec) && SharkHealth > 0) { // player bit shark on tail
					if (sharkBitten != 1) 
						Mix_PlayChannel(-1, fishBiteSound, 0);
					sharkBitten = 1;
				}

				// draw player fish
				SDL_Rect PosSize = { (int)playerPosition.x, (int)playerPosition.y, 16, 16 };
				if (playerDirection == 1) { // left
					SDL_BlitSurface(fish[playerRank], NULL, screen, &PosSize);
				}
				else { // right
					SDL_BlitSurface(fish[playerRank], NULL, screen, &PosSize);
				}

				// draw shark
				if (SharkSpawnTimer >= 900) {
					Mix_PlayChannel(-1, sharkSpawnSound, 0);
				}
				if (mrShark.active) {
					SDL_Rect GoTo = { (int)mrShark.position.x, (int)mrShark.position.y, 64, 32 };

					if (SharkHealth > 0) {
						sharkDeathAudioPlayed = 0;
						if (SharkHurtTimer % 10 && SharkHurtTimer > 0) {
							// color yellow eyes
						}
						else {
							// color red eyes
						}
						if (sharkDirection == 1) { // left
							SDL_BlitSurface(shark, NULL, screen, &GoTo);
						}
						else { // right
							SDL_BlitSurface(shark, NULL, screen, &GoTo);
						}
					}
					else {
						if (sharkDeathAudioPlayed == 0) {
							Mix_PlayChannel(-1, sharkDeadSound, 0);
							sharkDeathAudioPlayed = 1;
						}
						// restore eye color here: 
						if (sharkDirection == 1) { // left
							SDL_BlitSurface(shark_dead, NULL, screen, &GoTo);
						}
						else { // right
							SDL_BlitSurface(shark_dead, NULL, screen, &GoTo);
						}
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
								Mix_PlayChannel(-1, fishBiteSound, 0);
								Mix_PlayChannel(-1, fishBiteSound, 0);
								if (score % 1000 == 0 && playerRank < 4) {
									playerRank++;
									//printf("************** PLAYER RANK IS NOW %i ***************\n", playerRank);
									Mix_PlayChannel(-1, fishRankUp, 0);
								}
								else if (score % 1000 == 0 && playerRank >= 4) {
									lives++;
								}
							}
							else {
								//printf("**************** BITTEN BY A FISH. X:%f Y:%f ACTIVE:%i TYPE:%i ********************\n", creatures[i].position.x, creatures[i].position.y, creatures[i].active, creatures[i].type);
								PlayerBit();
								if (lives == 0)
									Mix_PlayChannel(-1, gameOverSound, 0);
								else
									Mix_PlayChannel(-1, deadSound, 0);
							}
						}

						SDL_Rect GoTo = { creatures[i].position.x, creatures[i].position.y, 16, 16 };
						SDL_Rect tmp = { animChange * 16, 0, ((animChange + 1) * 16), GoTo.y };
						if (creatures[i].type == 5) {
							SDL_BlitSurface(crab, NULL, screen, &GoTo);
						}
						else if (creatures[i].type == 6) {
							
							SDL_Rect tmp = { animChange * 16, 0, ((animChange + 1) * 16), GoTo.y };
							if (creatures[i].origin.x <= 20) {
								SDL_BlitSurface(lobster, NULL, screen, &GoTo);
							}
							else {
								SDL_BlitSurface(lobster, NULL, screen, &GoTo);
							}
						}
						else if (creatures[i].type <= 4 && creatures[i].type >= 0) {
							if (creatures[i].origin.x <= 20) {
								SDL_BlitSurface(fish[creatures[i].type], NULL, screen, &GoTo); // RIGHT
							}
							else {
								SDL_BlitSurface(fish[creatures[i].type], NULL, screen, &GoTo); // LEFT
							}
						}
						else if (creatures[i].type == 7) {
							if (creatures[i].origin.x <= 20) {
								SDL_BlitSurface(seahorse, NULL, screen, &GoTo);
							}
							else {
								SDL_BlitSurface(seahorse, NULL, screen, &GoTo);
							}
						}
						else if (creatures[i].type == 8) {
							SDL_BlitSurface(jellyfish, NULL, screen, &GoTo);
						}
					}
				}
				SDL_Flip(screen);

				SDL_Delay(time_left());
				next_time += TICK_INTERVAL;
			}
		}
	}
	Mix_FreeChunk(sharkSpawnSound);
	Mix_FreeChunk(sharkDeadSound);
	Mix_FreeChunk(fishBiteSound);
	Mix_FreeChunk(gameOverSound);
	Mix_FreeChunk(deadSound);
	Mix_FreeChunk(fishRankUp);

	SDL_FreeSurface(fish[0]);
	SDL_FreeSurface(fish[1]);
	SDL_FreeSurface(fish[2]);
	SDL_FreeSurface(fish[3]);
	SDL_FreeSurface(fish[4]);
	SDL_FreeSurface(shark);
	SDL_FreeSurface(shark_dead);
	SDL_FreeSurface(seahorse);
	SDL_FreeSurface(lobster);
	SDL_FreeSurface(crab);

	Mix_FreeMusic(bgMusic);
	TTF_CloseFont(font);
	Mix_CloseAudio();
	TTF_Quit();
	SDL_Quit(); // Quit SDL subsystems

	return 0;
}
