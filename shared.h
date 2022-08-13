#ifndef SHARED_H_   /* Include guard */
#define SHARED_H_

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
int lives;
int playerRank;
int playerDirection; // 1 = left, -1 = right
int FishSpawnTimer;
Shark mrShark;
int sharkBounces;
int sharkMaxBounces;
int score;
int SharkSpawnTimer;
int SharkHurtTimer;
int SharkHealth;
int LeftClick; // bool
int sharkDirection; // 1 = left, -1 = right
int PausedGame;
int mainMenu;
int GameOver;
int playerDead;
int sharkBitten; // bool
SeaCreature creatures[27];
extern int creatureRank[9];
extern float creatureSpeed[9];

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

#endif