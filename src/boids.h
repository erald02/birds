#ifndef BOIDS_H
#define BOIDS_H


#define FOV 200.0f
#define NUM_BOIDS 1000
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

void *worker_logic(void *arg);


#ifdef __cplusplus
extern "C" {
#endif

void init_cuda(boid_t* cpu_flock);
void step_physics_cuda(boid_t* cpu_flock);

#ifdef __cplusplus
}
#endif

#endif