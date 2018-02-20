ifeq ($(OS),Windows_NT)
MAIN = windows-main.c
CFLAGS = -fno-strict-aliasing
LDFLAGS = -mwindows -lopengl32 -lm -lpng
else
MAIN = gtk-main.c
CFLAGS = -std=gnu99 -fno-strict-aliasing $(shell pkg-config --cflags gtk+-2.0 gtkgl-2.0)
LDFLAGS = -lm -lpng $(shell pkg-config --libs gtk+-2.0 gtkgl-2.0)
endif

all: a.exe bomb.bin

clean :
	$(RM) a.exe bomb.bin

bomb.bin: bomb.c
	gcc -c -o bomb.o bomb.c
	objcopy -j.data bomb.o bomb.bin -O binary

a.exe: $(MAIN) model.c png.c gl.c gfx.c entity.c game.c
	gcc -o $@ $^ -W -Wall -Og -ggdb -fno-strict-aliasing $(CFLAGS) $(LDFLAGS)
