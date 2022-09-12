#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <sys/wait.h>
#include <time.h>
#include <semaphore.h>

int baseball_players = 0;
int soccer_players = 0;
int football_players = 0;

int activeSport = 0;

sem_t athlete_gen_semaphore;

void *generateAthlete(void* arg){
    int sport_val = *(int*) arg;
    char *sport = NULL;
    sem_wait(&athlete_gen_semaphore);
    if(sport_val == 2){
        soccer_players++;
        sport = "Soccer";
        printf("[Stadium]: Generating a soccer player, total number of players: %d\n", soccer_players);
        
    }
    else if(sport_val == 1){
        baseball_players++;
        sport = "Baseball";
        printf("[Stadium]; Generating a baseball player, total number of players: %d\n", baseball_players);
        
    }
    else if(sport_val == 0){
        football_players++;
        sport = "Football";
        printf("[Stadium]: Generating a football player, total number of players: %d\n", football_players);
        
    }
    else{
        printf("[Stadium]: Unable to create specified player\n");
    }
    sem_post(&athlete_gen_semaphore);
    int arrival_time = rand() % 4 + 1;
    sleep(arrival_time);
    printf("\t[%s Player]: I arrived to the complex in %d seconds\n", sport, arrival_time);
    return NULL;
}

int main(){
    printf("[Stadium]: Welcome to the OMNI-SPORTS complex\n");
    //seed creation
    FILE *fp;
    char *seed_file = "seed.txt";
    fp = fopen(seed_file, "r");
    if(fp == NULL){
        printf("File: %s, failed to open", seed_file);
        exit(EXIT_FAILURE);
    }
    int seed;
    fscanf(fp, "%d", &seed);
    printf("[Stadium]: Read seed: %d, From %s\n\n", seed , seed_file);
    srand(seed);

    sem_init(&athlete_gen_semaphore, 0, 1);
    pthread_t* thread_id = malloc(124 * sizeof(pthread_t));
    int counter = 0;
    
    for(int i = 0; i < 4; i++){
        int sport_val = rand() % 3;
        pthread_create(&thread_id[i], NULL, generateAthlete, &sport_val);
        counter++;
    }
    for(int i = 0; i < 4; i++){
        pthread_join(thread_id[i], NULL);
    }
    sem_destroy(&athlete_gen_semaphore);
    exit(0);
}