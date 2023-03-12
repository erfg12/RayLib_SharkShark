mkdir build
xcopy ..\assets .\assets /E /I /Y
emcc ../../shared.c ../SharkShark/main.c -s USE_SDL=2 -s USE_SDL_TTF=2 -s USE_SDL_MIXER=2 -s ALLOW_MEMORY_GROWTH=1 --preload-file assets -o build/game.html