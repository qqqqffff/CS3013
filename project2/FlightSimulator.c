#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>
#include <string.h>

#define MAX_LARGE_PLANES 15
#define MAX_SMALL_PLANES 30

int small_plane_count = 0;
int large_plane_count = 0;

pthread_mutex_t small_plane_mutex[MAX_SMALL_PLANES];
pthread_mutex_t large_plane_mutex[MAX_LARGE_PLANES];
pthread_cond_t small_plane_cond[MAX_SMALL_PLANES];
pthread_cond_t large_plane_cond[MAX_LARGE_PLANES];

sem_t airport_semaphore;


void *generatePlane(void *args){
    int plane_size = rand() % 2;
    int my_id = 0;
    char *plane_size_val = NULL;
    while(1){
        if((plane_size == 1) && (small_plane_count < MAX_SMALL_PLANES)){
            my_id = small_plane_count + 1001;
            pthread_mutex_init(&small_plane_mutex[small_plane_count]);
            plane_size_val = "Small";
            small_plane_count++;
            printf("[Airport]: %s plane has been generated, total number of small planes: %d", small_plane_count)
            break;
        }
        else if((plane_size == 1) && (small_plane_count > MAX_SMALL_PLANES)){

        }
        else if((plane_size == 0) && (large_plane_count < MAX_LARGE_PLANES)){
            break;
        }
        else if((plane_size == 0) && (large_plane_count > MAX_LARGE_PLANES)){

        }
    }
    return NULL;
}

int main(){
    FILE *fp;
    char *seed_file = "seed.txt";
    fp = fopen(seed_file, "r");
    if(fp == NULL){
        printf("File: %s, failed to open", seed_file);
        exit(EXIT_FAILURE);
    }
    int seed;
    fscanf(fp, "%d", &seed);
    printf("[Airport]: Read seed: %d, From %s\n\n", seed , seed_file);
    srand(seed);

//    sem_init(&airport_semaphore, 0, 1);
    pthread_t* planes = malloc((MAX_SMALL_PLANES + MAX_SMALL_PLANES) * sizeof(pthread_t));

    printf("[Airport]: Lets start these flights!\n");
    for(int i = 0; i < (MAX_LARGE_PLANES + MAX_SMALL_PLANES; i++){
        if((small_plane_count + large_plane_count) < (MAX_SMALL_PLANES + MAX_LARGE_PLANES)){
            pthread_create(&planes[i], NULL, generatePlane, NULL);
            usleep(100000);
        }
        else if((small_plane_count + large_plane_count) > (MAX_SMALL_PLANES + MAX_LARGE_PLANES)){
            break;
        }
    }

    for(int i = 0; i < (MAX_LARGE_PLANES + MAX_SMALL_PLANES); i++){
        printf("%d\n", pthread_join(athletes[i], NULL));
    }

}

