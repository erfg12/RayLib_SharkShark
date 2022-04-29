CFLAGS = -I. -I..\\..\allegro\include -I..
LDFLAGS = -L..\..\allegro\lib

SRC = main.c ../shared.c

all: main.c ../shared.c
	@echo BUILD... sharkshark.exe 
	gcc $(CFLAGS) $(LDFLAGS) $(SRC) -o sharkshark.exe -std=c11 -lalleg