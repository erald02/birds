
#include "cube.h"
#include "boids.h"
#include <SDL2/SDL_render.h>

const int CUBE_EDGES[12][2] = {{0, 1}, {1, 3}, {3, 2}, {2, 0}, {4, 5}, {5, 7},
                               {7, 6}, {6, 4}, {0, 4}, {1, 5}, {2, 6}, {3, 7}};

void render_cube(SDL_Renderer *renderer, camera_t *camera, cube_t *cube) {
  point_t square_screen[8];
  for (size_t i = 0; i < 8; i++) {
    float bx = cube->corners[i].x - camera->x;
    float by = cube->corners[i].y - camera->y;
    float bz = cube->corners[i].z - camera->z;

    square_screen[i].x = bx * cos(camera->yaw) - bz * sin(camera->yaw);
    float z_n = bx * sin(camera->yaw) + bz * cos(camera->yaw);

    square_screen[i].y = by * cos(camera->pitch) - z_n * sin(camera->pitch);
    square_screen[i].z = by * sin(camera->pitch) + z_n * cos(camera->pitch);
  }

  for (int i = 0; i < 12; i++) {
    int pt1 = CUBE_EDGES[i][0];
    int pt2 = CUBE_EDGES[i][1];

    float z1 = square_screen[pt1].z;
    float z2 = square_screen[pt2].z;

    if (z1 <= 0.1f && z2 <= 0.1f)
      continue;

    if (z1 <= 0.1f)
      z1 = 0.1f;
    if (z2 <= 0.1f)
      z2 = 0.1f;

    float z_factor1 = FOV / z1;
    int sx1 = (int)((square_screen[pt1].x * z_factor1) + (WINDOW_WIDTH / 2));
    int sy1 = (int)((square_screen[pt1].y * z_factor1) + (WINDOW_HEIGHT / 2));

    float z_factor2 = FOV / z2;
    int sx2 = (int)((square_screen[pt2].x * z_factor2) + (WINDOW_WIDTH / 2));
    int sy2 = (int)((square_screen[pt2].y * z_factor2) + (WINDOW_HEIGHT / 2));

    SDL_RenderDrawLine(renderer, sx1, sy1, sx2, sy2);
  }
}

const int OTHER_POINTS[7][3] = {{0, 0, 1}, {0, 1, 0}, {0, 1, 1}, {1, 0, 0},
                                {1, 0, 1}, {1, 1, 0}, {1, 1, 1}};

cube_t cube_from_coords(point_t center, int size) {
  cube_t cube;
  float half = size / 2.0f;
  int it = 0;

  for (int x = -1; x <= 1; x += 2) {
    for (int y = -1; y <= 1; y += 2) {
      for (int z = -1; z <= 1; z += 2) {
        cube.corners[it].x = center.x + (x * half);
        cube.corners[it].y = center.y + (y * half);
        cube.corners[it].z = center.z + (z * half);
        it++;
      }
    }
  }
  return cube;
}