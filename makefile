bird_asc: src/main.c
	gcc src/main.c src/physics.c src/cube.c -o birdys -pthread -lm -lSDL2