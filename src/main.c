#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <SDL2/SDL.h>

#define NUM_BOIDS 100

#define SIM_BOUNDS 100 
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define PI 3.14159265358979323846


typedef struct {
    int id;
    float x, y, z;
    float dx, dy, dz;    
    float weight;
    int* die;
} boid_t;

boid_t flock[NUM_BOIDS];
pthread_mutex_t locks[NUM_BOIDS];


float random_float(float min, float max) {
    float scale = (float)rand() / (float)RAND_MAX; 
    return min + scale * (max - min);
}


void* worker_logic(void* arg) {
    int my_id = *(int*)arg; 
    boid_t* me = &flock[my_id];
    
    while(1){
        if(*me->die == 0) break; 
        float adjust_dx = 0;
        float adjust_dy = 0;
        float adjust_dz = 0;
        int neighbors = 0;
        for (int i = 0; i < NUM_BOIDS; i++) {
            if (i == my_id) continue;
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
            float distance = sqrt(dist_x * dist_x + dist_y * dist_y + dist_z * dist_z);

            if (distance < 5.0) {
                neighbors++;
                
                if (distance < 1.5){
                    adjust_dx += dist_x * 0.15;
                    adjust_dy += dist_y * 0.15;
                    adjust_dz += dist_z * 0.15;
                }
                
                adjust_dx += other_dx * 0.02;
                adjust_dy += other_dy * 0.02;
                adjust_dz += other_dz * 0.02;
                
                adjust_dx -= dist_x * 0.01;
                adjust_dy -= dist_y * 0.01;
                adjust_dz -= dist_z * 0.01;
            }
        }

        pthread_mutex_lock(&locks[my_id]);
    
        me->dx += random_float(-0.02f, 0.02f);
        me->dy += random_float(-0.02f, 0.02f);
        me->dz += random_float(-0.02f, 0.02f);

        float speed = sqrt(me->dx*me->dx + me->dy*me->dy + me->dz*me->dz);
        float MAX_SPEED = 0.5f; 
        if (speed > MAX_SPEED) {
            me->dx = (me->dx / speed) * MAX_SPEED;
            me->dy = (me->dy / speed) * MAX_SPEED;
            me->dz = (me->dz / speed) * MAX_SPEED;
        }

        float nx = me->x + me->dx;
        if (nx > SIM_BOUNDS) { me->dx = -me->dx; nx = SIM_BOUNDS; }
        if (nx < 0) { me->dx = -me->dx; nx = 0; }
        
        float ny = me->y + me->dy;
        if (ny > SIM_BOUNDS) { me->dy = -me->dy; ny = SIM_BOUNDS; }
        if (ny < 0) { me->dy = -me->dy; ny = 0; }
        
        float nz = me->z + me->dz;
        if (nz > SIM_BOUNDS) { me->dz = -me->dz; nz = SIM_BOUNDS; }
        if (nz < 0) { me->dz = -me->dz; nz = 0; }
        
        me->x = nx;
        me->y = ny;
        me->z = nz;
        
        pthread_mutex_unlock(&locks[my_id]);
        
        usleep(10000);
    }
    return NULL;
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Birdys", 
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                                          WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);



    int num_threads = NUM_BOIDS;
    pthread_t threads[num_threads];
    int thread_ids[num_threads];
    srand(time(NULL));
    int running = 1;



    for (int i = 0; i < NUM_BOIDS; i++) {
        pthread_mutex_init(&locks[i], NULL);
        flock[i].id = i;
        flock[i].die = &running;
        flock[i].x = random_float(0, SIM_BOUNDS);
        flock[i].y = random_float(0, SIM_BOUNDS);
        flock[i].z = random_float(0, SIM_BOUNDS);
        flock[i].dx = random_float(-.2f, .2f);
        flock[i].dy = random_float(-.2f, .2f);
        flock[i].dz = random_float(-.2f, .2f);
    }

    SDL_Event event;
    float FOV = 400.0f;
    float CAMERA_Z = 70.0f;

    for (int i = 0; i < num_threads; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, worker_logic, &thread_ids[i]);
    }

    SDL_Rect outlineRect = { 0, 0, SIM_BOUNDS, SIM_BOUNDS};
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &outlineRect);
    SDL_RenderPresent(renderer);
    

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        float factor_back = FOV / (CAMERA_Z + SIM_BOUNDS);
        int back_w = (int)(SIM_BOUNDS * factor_back);
        int back_x = (int)((-SIM_BOUNDS / 2.0f) * factor_back) + (WINDOW_WIDTH / 2);
        int back_y = (int)((-SIM_BOUNDS / 2.0f) * factor_back) + (WINDOW_HEIGHT / 2);
        SDL_Rect back_rect = { back_x, back_y, back_w, back_w };

        float factor_front = FOV / CAMERA_Z;
        int front_w = (int)(SIM_BOUNDS * factor_front);
        int front_x = (int)((-SIM_BOUNDS / 2.0f) * factor_front) + (WINDOW_WIDTH / 2);
        int front_y = (int)((-SIM_BOUNDS / 2.0f) * factor_front) + (WINDOW_HEIGHT / 2);
        SDL_Rect front_rect = { front_x, front_y, front_w, front_w };

        SDL_SetRenderDrawColor(renderer, 150, 0, 0, 255); 

        SDL_RenderDrawRect(renderer, &back_rect);
        SDL_RenderDrawRect(renderer, &front_rect); 

        SDL_RenderDrawLine(renderer, front_x, front_y, back_x, back_y);                                 // Top Left
        SDL_RenderDrawLine(renderer, front_x + front_w, front_y, back_x + back_w, back_y);              // Top Right
        SDL_RenderDrawLine(renderer, front_x, front_y + front_w, back_x, back_y + back_w);              // Bottom Left
        SDL_RenderDrawLine(renderer, front_x + front_w, front_y + front_w, back_x + back_w, back_y + back_w); // Bottom Right


        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        for (size_t i = 0; i < NUM_BOIDS; i++) {
            pthread_mutex_lock(&locks[i]);
            float bx = flock[i].x - (SIM_BOUNDS / 2);
            float by = flock[i].y - (SIM_BOUNDS / 2);
            float bz = flock[i].z;
            pthread_mutex_unlock(&locks[i]);

            float z_factor = FOV / (CAMERA_Z + bz);
            int screen_x = (int)((bx * z_factor) + (WINDOW_WIDTH / 2));
            int screen_y = (int)((by * z_factor) + (WINDOW_HEIGHT / 2));
            int alpha = (int)(255.0f * (1.0f - (bz / SIM_BOUNDS)));
            if (alpha < 20) alpha = 20;   
            if (alpha > 255) alpha = 255;

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, alpha);

            if (screen_x >= 0 && screen_x < WINDOW_WIDTH && screen_y >= 0 && screen_y < WINDOW_HEIGHT) {
                SDL_Rect bird_rect = { screen_x, screen_y, 2, 2 };
                SDL_RenderFillRect(renderer, &bird_rect);
            }
        }

        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }

    printf("Shutting down cleanly...\n");

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
