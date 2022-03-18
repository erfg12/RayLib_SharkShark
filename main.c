#include "raylib.h"
#include <stdio.h>

const int screenWidth = 800;
const int screenHeight = 450;

typedef struct Shark {
    Vector2 origin;
    Vector2 position;
    Vector2 objective;
    Vector2 speed;

    bool active;
} Shark;

Vector2 playerPosition;
int SkippedFrames = 0;
Shark mrShark;

void SharkRoam() {
    if (mrShark.active)
    {
        float module;
        float sideX;
        float sideY;

        mrShark.objective = playerPosition;

        // Calculate speed
        module = sqrt(pow(mrShark.objective.x - mrShark.origin.x, 2) + pow(mrShark.objective.y - mrShark.origin.y, 2));

        sideX = (mrShark.objective.x - mrShark.origin.x) * 10/module;
        sideY = (mrShark.objective.y - mrShark.origin.y) * 10/module;

        mrShark.speed = (Vector2){ sideX, sideY };
    }
}

void SetVars() {
    playerPosition.x = (float)screenWidth / 2;
    playerPosition.y = (float)screenHeight / 2;

    mrShark.origin = (Vector2){0,0};
    mrShark.position = (Vector2){ 0,0 };
    mrShark.active = true;
}

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "Shark! Shark!");

    SetVars();
    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyDown(KEY_RIGHT) && playerPosition.x < screenWidth) playerPosition.x += 2.0f;
        if (IsKeyDown(KEY_LEFT) && playerPosition.x > 0) playerPosition.x -= 2.0f;
        if (IsKeyDown(KEY_UP) && playerPosition.y > 0) playerPosition.y -= 2.0f;
        if (IsKeyDown(KEY_DOWN) && playerPosition.y < screenHeight) playerPosition.y += 2.0f;

        struct Rectangle playerRec = { playerPosition.x, playerPosition.y, 20, 15 };
        //struct Rectangle sharkRec = { sharkPosition.x, sharkPosition.y, 50, 30 };

        //printf("DEBUG: player coords x:%f y:%f\n", ballPosition.x, ballPosition.y);
        //----------------------------------------------------------------------------------
        
        //if (SkippedFrames >= 2) {
            SharkRoam();
        //    SkippedFrames = 0;
        //}
        //SkippedFrames++;
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(BLUE);

            /*if (CheckCollisionRecs(playerRec, mrShark.position)) {
                DrawText("GAME OVER!", screenWidth / 2, screenHeight / 2, 20, RED);
            }*/

            DrawRectangle(playerPosition.x, playerPosition.y, 20, 15, YELLOW);
            DrawRectangle(mrShark.position.x, mrShark.position.y, 50, 30, BLACK);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

