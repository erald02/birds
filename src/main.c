#include "boids.h"
#include "cube.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

boid_t flock[NUM_BOIDS];
camera_t camera;

float random_float_r(unsigned int *seed, float min, float max) {
  float scale = (float)rand_r(seed) / (float)RAND_MAX;
  return min + scale * (max - min);
}

struct timespec start, end;

int main() {
  camera.x = SIM_BOUNDS / 2;
  camera.y = SIM_BOUNDS / 2;
  camera.z = -150;
  camera.yaw = 0;
  camera.pitch = 0;
  camera.roll = 0;

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL could not initialize! Error: %s\n", SDL_GetError());
    return 1;
  }

  SDL_Window *window =
      SDL_CreateWindow("Birdys", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  srand(time(NULL));
  int running = 1;
  unsigned int seed = time(NULL) ^ 1;
  for (int i = 0; i < NUM_BOIDS; i++) {
    flock[i].id = i;
    flock[i].x = random_float_r(&seed, 0, SIM_BOUNDS);
    flock[i].y = random_float_r(&seed, 0, SIM_BOUNDS);
    flock[i].z = random_float_r(&seed, 0, SIM_BOUNDS);
    flock[i].dx = random_float_r(&seed, -.2f, .2f);
    flock[i].dy = random_float_r(&seed, -.2f, .2f);
    flock[i].dz = random_float_r(&seed, -.2f, .2f);
  }

  SDL_Event event;

  int it = 0;
  cube_t cube;
  for (int x = 0; x <= 1; x++) {
    for (int y = 0; y <= 1; y++) {
      for (int z = 0; z <= 1; z++) {
        cube.corners[it].x = x ? SIM_BOUNDS : 0;
        cube.corners[it].y = y ? SIM_BOUNDS : 0;
        cube.corners[it].z = z ? SIM_BOUNDS : 0;
        it++;
      }
    }
  }

  Uint32 last_time = SDL_GetTicks();
  int frame_count = 0;
  char window_title[64];

  init_cuda(flock);

  while (running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = 0;
      }
    }
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    step_physics_cuda(flock);

    if (state[SDL_SCANCODE_W])
      camera.z += 2.0f;
    if (state[SDL_SCANCODE_S])
      camera.z -= 2.0f;
    if (state[SDL_SCANCODE_A])
      camera.x -= 2.0f;
    if (state[SDL_SCANCODE_D])
      camera.x += 2.0f;
    if (state[SDL_SCANCODE_SPACE])
      camera.y -= 2.0f;
    if (state[SDL_SCANCODE_LSHIFT])
      camera.y += 2.0f;

    if (state[SDL_SCANCODE_LEFT])
      camera.yaw -= 0.03f;
    if (state[SDL_SCANCODE_RIGHT])
      camera.yaw += 0.03f;
    if (state[SDL_SCANCODE_UP])
      camera.pitch -= 0.03f;
    if (state[SDL_SCANCODE_DOWN])
      camera.pitch += 0.03f;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 150, 0, 0, 255);
    render_cube(renderer, &camera, &cube);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (size_t i = 0; i < NUM_BOIDS; i++) {
      cube_t cube =
          cube_from_coords((point_t){flock[i].x, flock[i].y, flock[i].z}, 2);
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
      render_cube(renderer, &camera, &cube);
    }

    SDL_RenderPresent(renderer);
    frame_count++;
    Uint32 current_time = SDL_GetTicks();

    if (current_time - last_time >= 1000) {
      float fps = frame_count / ((current_time - last_time) / 1000.0f);

      printf("\rFPS: %6.1f | Boids: %d       ", fps, NUM_BOIDS);
      fflush(stdout);

      frame_count = 0;
      last_time = current_time;
    }
    // SDL_Delay(16);
  }

  printf("Shutting down cleanly...\n");

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
