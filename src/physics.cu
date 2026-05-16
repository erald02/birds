#include "boids.h"
#include <cuda_runtime.h>
#include <math.h>
#include <stdio.h>


__global__ void boids_kernel(boid_t *flock_in, boid_t *flock_out,
                             int num_boids) {
  int my_id = blockIdx.x * blockDim.x + threadIdx.x;

  if (my_id >= num_boids)
    return;

  boid_t me = flock_in[my_id];

  float adjust_dx = 0;
  float adjust_dy = 0;
  float adjust_dz = 0;
  int neighbors = 0;

  for (int i = 0; i < num_boids; i++) {
    if (i == my_id)
      continue;

    boid_t other = flock_in[i];

    float dist_x = me.x - other.x;
    float dist_y = me.y - other.y;
    float dist_z = me.z - other.z;
    float dist_sq = (dist_x * dist_x) + (dist_y * dist_y) + (dist_z * dist_z);

    if (dist_sq < 25.0f) {
      neighbors++;
      if (dist_sq < 2.25f) {
        adjust_dx += dist_x * 0.15f;
        adjust_dy += dist_y * 0.15f;
        adjust_dz += dist_z * 0.15f;
      }
      adjust_dx += other.dx * 0.02f;
      adjust_dy += other.dy * 0.02f;
      adjust_dz += other.dz * 0.02f;

      adjust_dx -= dist_x * 0.01f;
      adjust_dy -= dist_y * 0.01f;
      adjust_dz -= dist_z * 0.01f;
    }
  }

  if (neighbors > 0) {
    me.dx += (adjust_dx / neighbors);
    me.dy += (adjust_dy / neighbors);
    me.dz += (adjust_dz / neighbors);
  }

  float speed = sqrtf(me.dx * me.dx + me.dy * me.dy + me.dz * me.dz);
  float MAX_SPEED = 0.5f;
  if (speed > MAX_SPEED) {
    me.dx = (me.dx / speed) * MAX_SPEED;
    me.dy = (me.dy / speed) * MAX_SPEED;
    me.dz = (me.dz / speed) * MAX_SPEED;
  }

  me.x += me.dx;
  me.y += me.dy;
  me.z += me.dz;

  if (me.x > SIM_BOUNDS) {
    me.dx = -me.dx;
    me.x = SIM_BOUNDS;
  }
  if (me.x < 0) {
    me.dx = -me.dx;
    me.x = 0;
  }
  if (me.y > SIM_BOUNDS) {
    me.dy = -me.dy;
    me.y = SIM_BOUNDS;
  }
  if (me.y < 0) {
    me.dy = -me.dy;
    me.y = 0;
  }
  if (me.z > SIM_BOUNDS) {
    me.dz = -me.dz;
    me.z = SIM_BOUNDS;
  }
  if (me.z < 0) {
    me.dz = -me.dz;
    me.z = 0;
  }

  flock_out[my_id] = me;
}


boid_t *d_flock_in = NULL;
boid_t *d_flock_out = NULL;

extern "C" void init_cuda(boid_t *cpu_flock) {
  size_t size = NUM_BOIDS * sizeof(boid_t);
  cudaMalloc((void **)&d_flock_in, size);
  cudaMalloc((void **)&d_flock_out, size);

  cudaMemcpy(d_flock_in, cpu_flock, size, cudaMemcpyHostToDevice);
}

extern "C" void step_physics_cuda(boid_t *cpu_flock) {
  size_t size = NUM_BOIDS * sizeof(boid_t);

  int threadsPerBlock = 256;
  int blocksPerGrid = (NUM_BOIDS + threadsPerBlock - 1) / threadsPerBlock;

  boids_kernel<<<blocksPerGrid, threadsPerBlock>>>(d_flock_in, d_flock_out,
                                                   NUM_BOIDS);

  cudaDeviceSynchronize();

  cudaMemcpy(cpu_flock, d_flock_out, size, cudaMemcpyDeviceToHost);

  boid_t *temp = d_flock_in;
  d_flock_in = d_flock_out;
  d_flock_out = temp;
}