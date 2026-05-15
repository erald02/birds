#ifndef BOIDS_H
#define BOIDS_H

#include <pthread.h>

#define FOV 400.0f
#define NUM_BOIDS 500
#define SIM_BOUNDS 100
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define PI 3.14159265358979323846
#define SEED (int)time(NULL)

typedef struct {
  int id;
  float x, y, z;
  float dx, dy, dz;
  float weight;
  int *die;
} boid_t;

typedef struct {
  float x, y, z;
  float yaw, pitch, roll;
} camera_t;

extern boid_t flock[NUM_BOIDS];
extern pthread_mutex_t locks[NUM_BOIDS];

void *worker_logic(void *arg);

#endif