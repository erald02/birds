#ifndef CUBE_H
#define CUBE_H

#include "boids.h"
#include <SDL2/SDL_render.h>

typedef struct {
  float x, y, z;
} point_t;

typedef struct {
  point_t corners[8];
} cube_t;

void render_cube(SDL_Renderer *renderer, camera_t *camera, cube_t *cube);

cube_t cube_from_coords(point_t point, int size);

#endif
