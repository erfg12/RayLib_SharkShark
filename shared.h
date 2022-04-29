#ifndef SHARED_H_   /* Include guard */
#define SHARED_H_

#pragma once

typedef struct Vec2 {
    float x;
    float y;
} Vec2;

typedef struct Shark {
    Vec2 position;
    Vec2 objective;
    Vec2 speed;
    int active; // 0 = false, 1 = true
    int direction;
} Shark;

typedef struct SeaCreature {
    Vec2 position; // current positioning
    Vec2 origin; // helps determine which direction to head when spawned
    int active; // 0 = false, 1 = true
    int direction;
    int type; // 0-4 = fish ranked, 5 = seahorse, 6 = crab, 7 = lobster, 8 = jellyfish (use for creatureSize and creatureSpeed)
    int jump;
} SeaCreature;

Vec2 playerPosition;
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
int LeftClick = 0; // bool
int sharkDirection = 1; // 1 = left, -1 = right
int PausedGame = 0;
int GameOver = 0;
int playerDead = 0;
int sharkBitten = 0; // bool
SeaCreature creatures[27];
float creatureSpeed[9] = { 1, 1.3, 1.5, 1.7, 2, 0.8, 0.8, 2, 1.3 }; // use SC type to get speed
int creatureRank[9] = { 1, 2, 3, 4, 5, 5, 5, 5, 6 }; // use SC type to get rank. Rank determines what a creature can eat. (jellyfish are immune)

int GetRandomNum(int min, int max);
void SetShark();
void SetFish();
void SetVars(float ScreenWidth, float ScreenHeight);
void HurtShark();
void SharkRoam(float ScreenWidth, float ScreenHeight);
void FishSpawn(float ScreenWidth, float ScreenHeight);
void CrustJump(int CreatureID, float ScreenHeight);
void FishMoveAndDeSpawn(float ScreenWidth, float ScreenHeight);
void PlayerBit();

#endif // SHARED_H_