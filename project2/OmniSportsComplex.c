#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <sys/wait.h>
#include <time.h>
#include <semaphore.h>
#include <string.h>

#define MAX_BASEBALL_PLAYERS 36
#define MAX_SOCCER_PLAYERS 44
#define MAX_FOOTBALL_PLAYERS 44

pthread_mutex_t soccer_player_mutex[MAX_SOCCER_PLAYERS];
pthread_cond_t soccer_player_cond[MAX_SOCCER_PLAYERS];

pthread_mutex_t baseball_player_mutex[MAX_BASEBALL_PLAYERS];
pthread_cond_t baseball_player_cond[MAX_BASEBALL_PLAYERS];

pthread_mutex_t football_player_mutex[MAX_FOOTBALL_PLAYERS];
pthread_cond_t football_player_cond[MAX_FOOTBALL_PLAYERS];

#define MAX_BASEBALL_TEAM_COUNT 9
#define MIN_SOCCER_TEAM_COUNT 2
#define MAX_SOCCER_TEAM_COUNT 11
#define MAX_FOOTBALL_TEAM_COUNT 11

int baseball_players = 0;
int baseball_player_ids[MAX_BASEBALL_PLAYERS];
int soccer_players = 0;
int soccer_player_ids[MAX_SOCCER_PLAYERS];
int football_players = 0;
int football_player_ids[MAX_FOOTBALL_PLAYERS];

int ready_baseball_players = 0;
int ready_soccer_players = 0;
int ready_football_players = 0;

int active_sport = 0;
int *active_ids; 
char *active_sport_val = "None";
char *previous_field = "";

int ready_players_while_waiting = 0;
int blocked_players_while_waiting = 0;

sem_t athlete_semaphore;
sem_t game_semaphore;

int find_my_active_id(int my_id){
    for(int i = 0; i < sizeof(*active_ids); i++){
        int n = active_ids[i] - my_id - 10000;
        if(n == 0){
            return 1;
        }
        n -= 10000;
        if(n == 0){
            return 2;
        }
    }
    return -1;
}

void *generateAthlete(void* arg){
    int sport_val = rand() % 3;
    int my_id = 0;
    char *my_sport = NULL;
    sem_wait(&athlete_semaphore);
    //player generation
    while(1){
        if(sport_val == 2 && (soccer_players < MAX_SOCCER_PLAYERS)){
            my_id = soccer_players + 1001;
            soccer_player_ids[soccer_players] = my_id;
            pthread_mutex_init(&soccer_player_mutex[soccer_players], 0);
            pthread_cond_init(&soccer_player_cond[soccer_players], 0);
            soccer_players++;
            my_sport = "Soccer";
            if(!active_sport){
                printf("[Stadium]: Generating a soccer player, total number of players: %d\n", soccer_players);
            }
            break;
        }
        else if(sport_val == 2 && (soccer_players == MAX_SOCCER_PLAYERS)){
            if(!active_sport){
                printf("\t[Potential Soccer Player]: Theres too many other players, guess I\'ll play another day.\n");
            }
            else{
                blocked_players_while_waiting++;
            }
        }
        else if((sport_val == 1) && (baseball_players < MAX_BASEBALL_PLAYERS)){
            my_id = baseball_players + 2001;
            baseball_player_ids[baseball_players] = my_id;
            pthread_mutex_init(&baseball_player_mutex[baseball_players], 0);
            pthread_cond_init(&baseball_player_cond[baseball_players], 0);
            baseball_players++;
            my_sport = "Baseball";
            if(!active_sport){
                printf("[Stadium]: Generating a baseball player, total number of players: %d\n", baseball_players);
            }
            break;
        }
        else if(sport_val == 1 && (baseball_players == MAX_BASEBALL_PLAYERS)){
            if(!active_sport){
                printf("\t[Potential Baseball Player]: Theres too many other players, guess I\'ll play another day.\n");
            }
            else{
                blocked_players_while_waiting++;
            }
        }
        else if((sport_val == 0) && (football_players < MAX_FOOTBALL_PLAYERS)){
            my_id = football_players + 3001;
            football_player_ids[football_players] = my_id;
            pthread_mutex_init(&football_player_mutex[football_players], 0);
            pthread_cond_init(&football_player_cond[football_players], 0);
            football_players++;
            my_sport = "Football";
            if(!active_sport){
                printf("[Stadium]: Generating a football player, total number of players: %d\n", football_players);
            }
            break;
        }
        else if(sport_val == 0 && (football_players == MAX_FOOTBALL_PLAYERS)){
            if(!active_sport){
                printf("\t[Potential Football Player]: Theres too many other players, guess I\'ll play another day.\n");
            }
            else{
                blocked_players_while_waiting++;
            }
        }
        else{
            printf("[Stadium]: Unable to create specified player\n");
            return NULL;
        }
        sport_val = rand() % 3;
    }

    //Showing athlete arrival time
    sem_post(&athlete_semaphore);
    int arrival_time = rand() % 4 + 1;
    sleep(arrival_time);
    sem_wait(&athlete_semaphore);
    if(!active_sport){
        printf("\t[%s Player #%d]: I arrived to the complex in %d0 minutes!\n", my_sport, my_id, arrival_time);
    }
    else{
        ready_players_while_waiting++;
    }
    sem_post(&athlete_semaphore);

    //Printing total number of ready players
    sem_wait(&athlete_semaphore);
    if(sport_val == 2){
        ready_soccer_players++;
        if(!active_sport){
            printf("[Stadium]: Total number of ready %s players: %d\n", my_sport, ready_soccer_players);
        }
    }
    else if(sport_val == 1){
        ready_baseball_players++;
        if(!active_sport){
            printf("[Stadium]: Total number of ready %s players: %d\n", my_sport, ready_baseball_players);
        }
    }
    else if(sport_val == 0){
        ready_football_players++;
        if(!active_sport){
            printf("[Stadium]: Total number of ready %s players: %d\n", my_sport, ready_football_players);
        }
    }
    sem_post(&athlete_semaphore);

    //TODO: wait until called and active sport
    while(1){
        //waiting for their team
        sem_wait(&athlete_semaphore);
        if(!active_sport){
            printf("\t[%s Player #%d]: I will wait for my turn. . . This is might take a while\n", my_sport, my_id);
        }
        sem_post(&athlete_semaphore);
        char *my_team = NULL;
        if(strcmp(my_sport, "Soccer") == 0){
            pthread_cond_wait(&soccer_player_cond[my_id - 1001], &soccer_player_mutex[my_id - 1001]);
        }
        else if(strcmp(my_sport, "Baseball") == 0){
            // printf("%d, ", pthread_mutex_lock(&baseball_player_mutex[my_id - 2001]));
            pthread_cond_wait(&baseball_player_cond[my_id - 2001], &baseball_player_mutex[my_id - 2001]);
            int search_res = find_my_active_id(my_id);
            if(search_res == 1){
                my_team = "A";
            }
            else if(search_res == 0){
                my_team = "B";
            }
            else{
                printf("This isnt supposed to happen");
            }
        }
        else if(strcmp(my_sport, "Football") == 0){
            pthread_cond_wait(&football_player_cond[my_id - 3001], &football_player_mutex[my_id - 3001]);
        }

        printf("\t[%s Player #%d]: I have been called upon to play for team %s\n", my_sport, my_id, my_team);
        if(strcmp(my_sport, "Soccer") == 0){
            pthread_cond_wait(&soccer_player_cond[my_id - 1001], &soccer_player_mutex[my_id - 1001]);
        }
        else if(strcmp(my_sport, "Baseball") == 0){
            // printf("%d, ", pthread_mutex_lock(&baseball_player_mutex[my_id - 2001]));
            pthread_cond_wait(&baseball_player_cond[my_id - 2001], &baseball_player_mutex[my_id - 2001]);
        }
        else if(strcmp(my_sport, "Football") == 0){
            pthread_cond_wait(&football_player_cond[my_id - 3001], &football_player_mutex[my_id - 3001]);
        }
        printf("\t[");
        break;
    }
    // while(!active_sport);
    return NULL;
}

void *simulateBaseballGame(void* args){
    printf("\n[Stadium]: Starting the BASEBALL GAME!\n");
    printf("[Baseball]: Generating \"balanced\" teams\n");
    
    int *teamA_ids = malloc(MAX_BASEBALL_TEAM_COUNT * sizeof(int));
    int *teamB_ids = malloc(MAX_BASEBALL_TEAM_COUNT * sizeof(int));
    active_ids = malloc(MAX_BASEBALL_TEAM_COUNT * 2  * sizeof(int));
    sem_wait(&athlete_semaphore);
    printf("[Baseball]: Total number of players to choose from: %d\n", ready_baseball_players);
    //team assigning todo: figure out how to parse ids
    int j = 0;
    for(int i = 0; i < MAX_BASEBALL_TEAM_COUNT; i++){
        int location = rand() % ready_baseball_players;
        for(int j = 0; j < i; j++){
            if(teamA_ids[j] == baseball_player_ids[location] || teamB_ids[j] == baseball_player_ids[location]){
                location = rand() % ready_baseball_players;
                j = 0;
            }
        }
        active_ids[j] = baseball_player_ids[location] + 10000; j++;
        teamA_ids[i] = baseball_player_ids[location];
        pthread_cond_signal(&baseball_player_cond[teamA_ids[i] - 2001]);
        printf("[Baseball]: Player #%d has joined team A!\n", teamA_ids[i]);
        location = rand() % ready_baseball_players;
        for(int j = 0; j < i; j++){
            if(teamB_ids[j] == baseball_player_ids[location] || teamA_ids[j] == baseball_player_ids[location]){
                location = rand() % ready_baseball_players;
                j = 0;
            }
        }
        active_ids[j] = baseball_player_ids[location] + 20000; j++;
        teamB_ids[i] = baseball_player_ids[location];
        pthread_cond_signal(&baseball_player_cond[teamB_ids[i] - 2001]);
        printf("[Baseball]: Player #%d has joined team B!\n", teamB_ids[i]);
        usleep(50000); //between team asignments .05 seconds
    }
    sem_post(&athlete_semaphore);
    usleep(1000);
    printf("[Baseball]: Flipping a coin for batting team");
    sleep(10);
    printf(". ");
    // usleep(500000);
    // printf(". ");
    // usleep(500000);
    // printf(". ");
    // usleep(500000);
    printf("TAILS!\n");
    int batting_team = rand() % 2;
    //batting team A = 0, B = 1

    int teamAscore = 0;
    int teamBscore = 0;
    // for(int i = 1; i < 10; i++){
    //     int inning_time = rand() % 4 + 1;
    //     int bscore_randomizer = rand() % 100 + 1;
        
    //     //determining the inning
    //     if(i == 1){
    //         if(batting_team == 0){
    //             printf("[Baseball]: Its the start of the %dst and Team %s is up to bat\n", i, "A");
    //         }
    //         else{
    //             printf("[Baseball]: Its the start of the %dst and Team %s is up to bat\n", i, "B");
    //         }
    //     }
    //     else if(i == 2){
    //         if(batting_team == 0){
    //             printf("[Baseball]: Its the start of the %dnd and Team %s is up to bat\n", i, "A");
    //         }
    //         else{
    //             printf("[Baseball]: Its the start of the %dnd and Team %s is up to bat\n", i, "B");
    //         }
    //     }
    //     else if(i == 3){
    //         if(batting_team == 0){
    //             printf("[Baseball]: Its the start of the %drd and Team %s is up to bat\n", i, "A");
    //         }
    //         else{
    //             printf("[Baseball]: Its the start of the %drd and Team %s is up to bat\n", i, "B");
    //         }
    //     }
    //     else{
    //         if(batting_team == 0){
    //             printf("[Baseball]: Its the start of the %dth and Team %s is up to bat\n", i, "A");
    //         }
    //         else{
    //             printf("[Baseball]: Its the start of the %dth and Team %s is up to bat\n", i, "B");
    //         }
    //     }
        
    //     sleep(inning_time);
    //     //score handling
    //     if(bscore_randomizer >= 95){
    //         if(batting_team == 0){
    //             teamAscore += 3;
    //             printf("[Baseball]: With loaded bases team %s hit a grand slam!\n", "A");
    //             printf("[Baseball]: Total score for team %s: %d\n", "A", teamAscore);
    //         }
    //         else{
    //             teamBscore += 3;
    //             printf("[Baseball]: With loaded bases team %s hit a grand slam!\n", "B");
    //             printf("[Baseball]: Total score for team %s: %d\n", "B", teamBscore);
    //         }
    //     }
    //     else if(bscore_randomizer < 95 && bscore_randomizer >= 85){
    //         if(batting_team == 0){
    //             teamAscore += 2;
    //             printf("[Baseball]: Team %s gets away with 2 runs\n", "A");
    //             printf("[Baseball]: Total score for team %s: %d\n", "A", teamAscore);
    //         }
    //         else{
    //             teamAscore += 2;
    //             printf("[Baseball]: Team %s gets away with 2 runs\n", "B");
    //             printf("[Baseball]: Total score for team %s: %d\n", "B", teamBscore);
    //         }
    //     }
    //     else if(bscore_randomizer < 85 && bscore_randomizer >= 65){
    //         if(batting_team == 0){
    //             teamAscore += 1;
    //             printf("[Baseball]: Team %s only gets 1 runs\n", "A");
    //             printf("[Baseball]: Total score for team %s: %d\n", "A", teamAscore);
    //         }
    //         else{
    //             teamAscore += 1;
    //             printf("[Baseball]: Team %s only gets 1 runs\n", "B");
    //             printf("[Baseball]: Total score for team %s: %d\n", "B", teamBscore);
    //         }
    //     }
    //     else{
    //         if(batting_team == 0){
    //             printf("[Baseball]: Team %s has been SHUTOUT\n", "A");
    //             printf("[Baseball]: Total score for team %s: %d\n", "A", teamAscore);
    //         }
    //         else{
    //             printf("[Baseball]: Team %s has been SHUTOUT\n", "B");
    //             printf("[Baseball]: Total score for team %s: %d\n", "B", teamBscore);
    //         }
    //     }
        
    //     if(batting_team == 0){
    //         printf("[Baseball]: Team %s has finished thier turn with an inning time of %d0 minutes!\n", "A", inning_time);
    //         batting_team = 1;
    //     }
    //     else{
    //         batting_team = 0;
    //     }
    // }
    return NULL;
}
void *simulateSoccerGame(void* args){
    return NULL;
}
void *simulateFootballGame(void* args){
    return NULL;
}

int main(){
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

    printf("[Stadium]: Welcome to the OMNI-SPORTS complex\n");
    printf("[Stadium]: Our field is first come, first SERVE!\n");

    sem_init(&athlete_semaphore, 0, 1);
    pthread_t* athletes = malloc(124 * sizeof(pthread_t));
    pthread_t sports_match;
    int counter = 0;
    //insert a while true when done testing
    for(int i = 0; (ready_baseball_players + ready_football_players + ready_soccer_players) < (MAX_BASEBALL_PLAYERS + MAX_FOOTBALL_PLAYERS + MAX_SOCCER_PLAYERS); i++){
        //player generation
        pthread_create(&athletes[i], NULL, generateAthlete, NULL);
        usleep(100000); //pausing .1 seconds between player generation to prevent overlap
        counter++;

        //start baseball simulation
        if((ready_baseball_players > 18) && (strcmp(previous_field, "BASEBALL") != 0)){
            active_sport = 1;
            active_sport_val = "BASEBALL";
            previous_field = "BASEBALL";
            printf("\n\n[Stadium]: Ready for a game of BALL?, Field Configuration: %s\n", active_sport_val);
            printf("[Stadium]: Make sure that there are no SOCCER players or FOOTBALL players on the field\n");
            if(fork() == 0){
                pthread_create(&sports_match, NULL, simulateBaseballGame, NULL);
                pthread_join(sports_match, NULL);
                exit(0);
            }
            break;
        }
        //Start football simulation
        // else if((ready_football_players > 22) && (strcmp(previous_field, "FOOTBALL") != 0)){

        // }
        //Start soccer simulation
        // else if((ready_soccer_players > 4) && (strcmp(previous_field, "SOCCER") != 0)){

        // }
    }
    for(int i = 0; i < counter; i++){
        pthread_join(athletes[i], NULL);
    }
    sem_destroy(&athlete_semaphore);
    exit(0);
}