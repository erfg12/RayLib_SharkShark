OBJS = ../../shared.c main.c
CC = g++
COMPILER_FLAGS = -w
LINKER_FLAGS = -lSDL2 -lSDL2main -lSDL2_ttf -lSDL2_mixer
INC=-I../SDL_libs/SDL2-2.26.3 -I../SDL_libs/SDL2_ttf-2.20.2 -I../SDL_libs/SDL2_mixer-2.6.3
OBJ_NAME = SharkShark

all : $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) $(INC) -o $(OBJ_NAME)
	cp -R '../assets' './'