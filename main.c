#include "raylib.h"
#include <stdio.h>

const int screenWidth = 800;
const int screenHeight = 450;

Vector2 sharkPosition;
Vector2 playerPosition;
int SkippedFrames = 0;

void SharkRoam() {
    sharkPosition.x--;
}

void SetVars() {
    playerPosition.x = (float)screenWidth / 2;
    playerPosition.y = (float)screenHeight / 2;

    sharkPosition.x = (float)screenWidth - 20;
    sharkPosition.y = 20;
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
        struct Rectangle sharkRec = { sharkPosition.x, sharkPosition.y, 50, 30 };

        //printf("DEBUG: player coords x:%f y:%f\n", ballPosition.x, ballPosition.y);
        //----------------------------------------------------------------------------------
        
        if (SkippedFrames >= 2) {
            SharkRoam();
            SkippedFrames = 0;
        }
        SkippedFrames++;
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(BLUE);

            if (CheckCollisionRecs(playerRec, sharkRec)) {
                DrawText("GAME OVER!", screenWidth / 2, screenHeight / 2, 20, RED);
            }

            DrawRectangle(playerPosition.x, playerPosition.y, 20, 15, YELLOW);
            DrawRectangle(sharkPosition.x, sharkPosition.y, 50, 30, BLACK);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

