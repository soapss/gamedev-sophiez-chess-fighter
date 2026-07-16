CC=clang
CFLAGS=-O2 -Wall -I/tmp/raylib-5.0/src
LDFLAGS=/tmp/raylib-5.0/src/libraylib.a -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL -lm
SRC=main.c
OUT=../TidEscape

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)

clean:
	rm -f $(OUT)
