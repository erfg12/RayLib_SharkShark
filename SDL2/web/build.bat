mkdir build
xcopy ..\assets .\assets /E /I /Y
emcc -O3 ../../shared.c ../SharkShark/main.c -pthread -sUSE_SDL=2 -sUSE_SDL_TTF=2 -sUSE_SDL_MIXER=2 -sALLOW_MEMORY_GROWTH=1 --preload-file assets -o build/index.html -O3 --shell-file shell.html
