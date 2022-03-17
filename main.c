/*******************************************************************************************
*
*   raylib [core] example - Keyboard input
*
*   This example has been created using raylib 1.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2014 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include <stdio.h>

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib Shark! Shark!");

    Vector2 playerPosition = { (float)screenWidth/2, (float)screenHeight/2 };
    Vector2 sharkPosition = { (float)screenWidth-30, 10};

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

        struct Rectangle playerRec;
        strcpy(playerRec.x, playerPosition.x);
        strcpy(playerRec.y, playerPosition.x);
        strcpy(playerRec.width, 20);
        strcpy(playerRec.height, 15);

        struct Rectangle sharkRec;
        strcpy(sharkRec.x, sharkPosition.x);
        strcpy(sharkRec.y, sharkPosition.y);
        strcpy(sharkRec.width, 20);
        strcpy(sharkRec.height, 15);

        //printf("DEBUG: player coords x:%f y:%f\n", ballPosition.x, ballPosition.y);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            if (CheckCollisionRecs(playerRec, sharkRec)) {
                DrawText("GAME OVER!", screenWidth / 2, screenHeight / 2, 20, RED);
            }

            DrawRectangle(playerPosition.x, playerPosition.y, 20, 15, BLUE);
            DrawRectangle(sharkPosition.x, sharkPosition.y, 50, 30, DARKBLUE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}