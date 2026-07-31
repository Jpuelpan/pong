CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c99 `pkg-config --cflags --libs sdl3`

build: main.c
	$(CC) $(CFLAGS) -g -o pong main.c

build-wasm: main.c
	emcc --use-port=sdl3 -o web/pong.html main.c # --embed-file ./assets/@/assets/

clean:
	rm ./pong

run: build
	./pong

debug: build
	gdb ./pong
