bird_asc: src/main.c
	nvcc -std=c++20 src/main.c src/cube.c src/physics.cu -o birdys -lSDL2 -lm