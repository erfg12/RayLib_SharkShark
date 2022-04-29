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



void init() {
    int depth, res;
    allegro_init();
    depth = desktop_color_depth();
    if (depth == 0) depth = 32;
    set_color_depth(depth);
#ifdef _WIN32
    res = set_gfx_mode(GFX_AUTODETECT_WINDOWED, X * P, Y * P, 0, 0);
#else
    res = set_gfx_mode(GFX_AUTODETECT, 320, 200, 0, 0);
#endif
    if (res != 0) {
        allegro_message(allegro_error);
        exit(-1);
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

    while (!key[KEY_ESC]) {
        // 
    }

    deinit();
    return 0;
}