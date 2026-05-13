#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

#define NUM_BOIDS 100
#define SCREEN_X 200
#define SCREEN_Y 40
#define PI 3.14159265358979323846


typedef struct {
    int id;
    float x, y;
    float dx, dy;    
    float weight; // influence on other birds
    int die;
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
        float adjust_dx = 0;
        float adjust_dy = 0;
        int neighbors = 0;
        for (int i = 0; i < NUM_BOIDS; i++) {
            if (i == my_id) continue;
            pthread_mutex_lock(&locks[i]);
            float other_x = flock[i].x;
            float other_y = flock[i].y;
            float other_dx = flock[i].dx;
            float other_dy = flock[i].dy;
            pthread_mutex_unlock(&locks[i]); 
            float dist_x = me->x - other_x;
            float dist_y = me->y - other_y;
            float distance = sqrt(dist_x * dist_x + dist_y * dist_y);

            if (distance < 5.0) {
                neighbors++;
                
                if (distance < 1.5){
                    adjust_dx += dist_x * 0.1;
                    adjust_dy += dist_y * 0.1;
                }
                
                adjust_dx += other_dx * 0.02;
                adjust_dy += other_dy * 0.02;
                
                adjust_dx -= dist_x * 0.01;
                adjust_dy -= dist_y * 0.01;
            }
        }
        pthread_mutex_lock(&locks[my_id]);
        float nx, ny;
        if (neighbors > 0) {
            me->dx += (adjust_dx / neighbors);
            me->dy += (adjust_dy / neighbors);
        }

        me->dx += random_float(-0.002f, 0.002f);
        me->dy += random_float(-0.002f, 0.002f);

        float speed = sqrt(me->dx * me->dx + me->dy * me->dy);
        float MAX_SPEED = 0.05f; 
        if (speed > MAX_SPEED) {
            me->dx = (me->dx / speed) * MAX_SPEED;
            me->dy = (me->dy / speed) * MAX_SPEED;
        }

        nx = me->x + me->dx;
        if (nx > SCREEN_X){
            me->dx = -me->dx;
            nx = SCREEN_X - (nx - SCREEN_X);
        }
        if (nx < 0){
            me->dx = -me->dx;
            nx = -nx;
        }
        ny = me->y + me->dy;
        if (ny > SCREEN_Y){
            me->dy = -me->dy;
            ny = SCREEN_Y - (ny - SCREEN_Y);
        }
        if (ny < 0){
            me->dy = -me->dy;
            ny = -ny;
        }
        me->x = nx;
        me->y = ny;
        pthread_mutex_unlock(&locks[my_id]);
        if(me->die == 1){
            break;
        }
        usleep(5000);
    }
    return NULL;
}

void* render_screen(void* arg){
    while(1) {
        printf("\033[H");
        
        double screen[SCREEN_Y][SCREEN_X];
        for(size_t y = 0; y < SCREEN_Y; y++) {
            for(size_t x = 0; x < SCREEN_X; x++) {
                screen[y][x] = -1.0; 
            }
        }

        for(size_t i = 0; i < NUM_BOIDS; i++){
            pthread_mutex_lock(&locks[i]);
            double rad = atan2(-flock[i].dy, flock[i].dx);
            
            if (rad < 0) {
                rad += 2 * PI;
            }
            
            int cy = (int)flock[i].y;
            int cx = (int)flock[i].x;
            if (cx >= 0 && cx < SCREEN_X && cy >= 0 && cy < SCREEN_Y) {
                screen[cy][cx] = rad;
            }
            pthread_mutex_unlock(&locks[i]);
        }

        for(size_t y = 0; y < SCREEN_Y; y++){
            for(size_t x = 0; x < SCREEN_X; x++){
                if(screen[y][x] >= 0){ 
                    double shifted = screen[y][x] + (PI / 8.0);
                    if (shifted >= 2 * PI) shifted -= 2 * PI;

                    int sector = (int)(shifted / (PI / 4.0));
                    

                    switch (sector) {
                        case 0: printf(">");  break;
                        case 1: printf("/");  break; 
                        case 2: printf("^");  break;
                        case 3: printf("\\"); break; 
                        case 4: printf("<");  break; 
                        case 5: printf("/");  break; 
                        case 6: printf("v");  break; 
                        case 7: printf("\\"); break; 
                        default: printf("?"); break;
                    }
                } else {
                    printf("."); 
                }
            }
            printf("\n");
        }
        usleep(16500); 
    }
    return NULL;
}

int main() {
    printf("\033[?25l");
    int num_threads = NUM_BOIDS;
    pthread_t threads[num_threads];
    pthread_t screen_thread;
    int thread_ids[num_threads];

    srand(time(NULL));

    for (int i = 0; i < NUM_BOIDS; i++) {
        pthread_mutex_init(&locks[i], NULL);
        flock[i].id = i;
        
        flock[i].x = random_float(0, SCREEN_X);
        flock[i].y = random_float(0, SCREEN_Y);
        
        flock[i].dx = random_float(-0.1f, 0.1f);
        flock[i].dy = random_float(-0.1f, 0.1f);
    }

    for (int i = 0; i < num_threads; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, worker_logic, &thread_ids[i]);
    }

    pthread_create(&screen_thread, NULL, render_screen, NULL);

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_join(screen_thread, NULL);
    printf("\033[?25h");
    return 0;
}
