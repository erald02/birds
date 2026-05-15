#include "boids.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

float random_float_r(unsigned int *seed, float min, float max) {
  float scale = (float)rand_r(seed) / (float)RAND_MAX;
  return min + scale * (max - min);
}

void *worker_logic(void *arg) {
  int my_id = *(int *)arg;
  boid_t *me = &flock[my_id];

  unsigned int seed = time(NULL) ^ my_id;

  while (1) {
    if (*me->die == 0)
      break;

    float adjust_dx = 0;
    float adjust_dy = 0;
    float adjust_dz = 0;
    int neighbors = 0;

    for (int i = 0; i < NUM_BOIDS; i++) {
      if (i == my_id)
        continue;

      pthread_mutex_lock(&locks[i]);
      float other_x = flock[i].x;
      float other_y = flock[i].y;
      float other_z = flock[i].z;
      float other_dx = flock[i].dx;
      float other_dy = flock[i].dy;
      float other_dz = flock[i].dz;
      pthread_mutex_unlock(&locks[i]);

      float dist_x = me->x - other_x;
      float dist_y = me->y - other_y;
      float dist_z = me->z - other_z;

      float dist_sq = (dist_x * dist_x) + (dist_y * dist_y) + (dist_z * dist_z);

      if (dist_sq < 25.0f) { // 5^2
        neighbors++;

        if (dist_sq < 2.25f) { // 1.5^2
          adjust_dx += dist_x * 0.15f;
          adjust_dy += dist_y * 0.15f;
          adjust_dz += dist_z * 0.15f;
        }

        adjust_dx += other_dx * 0.02f;
        adjust_dy += other_dy * 0.02f;
        adjust_dz += other_dz * 0.02f;

        adjust_dx -= dist_x * 0.01f;
        adjust_dy -= dist_y * 0.01f;
        adjust_dz -= dist_z * 0.01f;
      }
    }

    pthread_mutex_lock(&locks[my_id]);

    if (neighbors > 0) {
      me->dx += (adjust_dx / neighbors);
      me->dy += (adjust_dy / neighbors);
      me->dz += (adjust_dz / neighbors);
    }

    me->dx += random_float_r(&seed, -0.02f, 0.02f);
    me->dy += random_float_r(&seed, -0.02f, 0.02f);
    me->dz += random_float_r(&seed, -0.02f, 0.02f);

    float speed = sqrt(me->dx * me->dx + me->dy * me->dy + me->dz * me->dz);
    float MAX_SPEED = 0.5f;
    if (speed > MAX_SPEED) {
      me->dx = (me->dx / speed) * MAX_SPEED;
      me->dy = (me->dy / speed) * MAX_SPEED;
      me->dz = (me->dz / speed) * MAX_SPEED;
    }

    float nx = me->x + me->dx;
    if (nx > SIM_BOUNDS) {
      me->dx = -me->dx;
      nx = SIM_BOUNDS;
    }
    if (nx < 0) {
      me->dx = -me->dx;
      nx = 0;
    }

    float ny = me->y + me->dy;
    if (ny > SIM_BOUNDS) {
      me->dy = -me->dy;
      ny = SIM_BOUNDS;
    }
    if (ny < 0) {
      me->dy = -me->dy;
      ny = 0;
    }

    float nz = me->z + me->dz;
    if (nz > SIM_BOUNDS) {
      me->dz = -me->dz;
      nz = SIM_BOUNDS;
    }
    if (nz < 0) {
      me->dz = -me->dz;
      nz = 0;
    }

    me->x = nx;
    me->y = ny;
    me->z = nz;

    pthread_mutex_unlock(&locks[my_id]);

    usleep(10000);
  }
  return NULL;
}