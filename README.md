# CUDA Boids
cuda boids birds

## Reqs
* CUDA Toolkit (`nvcc`)
* SDL2 (`libsdl2-dev`)
* `gcc`

## Run
```bash
nvcc -std=c++20 src/main.c src/cube.c src/physics.cu -o birdys -lSDL2 -lm
#or
make

./birdys