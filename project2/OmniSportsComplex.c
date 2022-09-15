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
int active_sport_completed = 0;
int *active_ids; 
char *active_sport_val = "None";
int previous_field[2];
int prev_field = 0;

int ready_players_while_waiting = 0;
int blocked_players_while_waiting = 0;

sem_t athlete_semaphore;
sem_t game_semaphore;

int prev_contained(int sport){
    for(int i = 0; i < 2; i++){
        if(previous_field[i] == sport){
            return -1;
        }
    }
    return 0;
}
int find_my_active_id(int my_id){
    int counter = 0;
    while(active_ids[counter] != 0){
        int n = active_ids[counter] - my_id - 10000;
        if(n == 0){
            return 1;
        }
        n -= 10000;
        if(n == 0){
            return 0;
        }
        if(counter > 44){
            break;
        }
        counter++;
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
    if(active_sport == 0){
        printf("\t[%s Player #%d]: I arrived to the complex in %d0 minutes!\n", my_sport, my_id, arrival_time);
    }
    else if(active_sport == 1){
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
        if(sport_val == 2){
            pthread_cond_wait(&soccer_player_cond[my_id - 1001], &soccer_player_mutex[my_id - 1001]);
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
        else if(sport_val == 1){
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
        else if(sport_val == 0){
            pthread_cond_wait(&football_player_cond[my_id - 3001], &football_player_mutex[my_id - 3001]);
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

        usleep(10000);
        printf("\t[%s Player #%d]: I have been called upon to play for team %s\n", my_sport, my_id, my_team);

        //in game!
        if(strcmp(my_sport, "Soccer") == 0){
            pthread_cond_wait(&soccer_player_cond[my_id - 1001], &soccer_player_mutex[my_id - 1001]);
        }
        else if(strcmp(my_sport, "Baseball") == 0){
            pthread_cond_wait(&baseball_player_cond[my_id - 2001], &baseball_player_mutex[my_id - 2001]);
        }
        else if(strcmp(my_sport, "Football") == 0){
            pthread_cond_wait(&football_player_cond[my_id - 3001], &football_player_mutex[my_id - 3001]);
        }

        //game finish
        if(strcmp(my_team, "A") == 0){
            printf("\t[%s Player #%d]: Good game team %s\n", my_sport, my_id, "B");
        }
        else if(strcmp(my_team, "B") == 0){
            printf("\t[%s Player #%d]: Good game team %s\n", my_sport, my_id, "A");
        }
        usleep(100000);
        printf("\t[%s Player #%d]: I will return to waiting this may take a while...\n", my_sport, my_id);
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
    sem_wait(&athlete_semaphore);
    active_ids = malloc(MAX_BASEBALL_TEAM_COUNT * 2  * sizeof(int));
    
    // ready_baseball_players -= 18;
    printf("[Baseball]: Total number of players to choose from: %d\n", ready_baseball_players);
    //team assigning todo: figure out how to parse ids
    int j = 0;
    
    for(int i = 0; i < MAX_BASEBALL_TEAM_COUNT; i++){
        int location = rand() % ready_baseball_players;
        for(int k = 0; k <= i; k++){
            if(teamA_ids[k] == baseball_player_ids[location] || teamB_ids[k] == baseball_player_ids[location]){
                location = rand() % ready_baseball_players;
                k = 0;
            }
        }
        
        sem_wait(&athlete_semaphore);
        active_ids[j] = baseball_player_ids[location] + 10000; j++;
        sem_post(&athlete_semaphore);
        
        teamA_ids[i] = baseball_player_ids[location];
        pthread_cond_signal(&baseball_player_cond[teamA_ids[i] - 2001]);
        printf("[Baseball]: Player #%d has joined team A!\n", teamA_ids[i]);

        location = rand() % ready_baseball_players;
        for(int k = 0; k <= i; k++){
            if(teamB_ids[k] == baseball_player_ids[location] || teamA_ids[k] == baseball_player_ids[location]){
                location = rand() % ready_baseball_players;
                k = 0;
            }
        }
        
        sem_wait(&athlete_semaphore);
        active_ids[j] = baseball_player_ids[location] + 20000; j++;
        sem_post(&athlete_semaphore);
        
        teamB_ids[i] = baseball_player_ids[location];
        pthread_cond_signal(&baseball_player_cond[teamB_ids[i] - 2001]);
        
        printf("[Baseball]: Player #%d has joined team B!\n", teamB_ids[i]);
        usleep(50000); //between team asignments .05 seconds
    }
    sem_post(&athlete_semaphore);
    sleep(1);
    printf("[Baseball]: Flipping a coin for batting team. . . ");
    int batting_team = rand() % 2;
    if(batting_team == 0){
        printf("HEADS!\n\n");
    }
    else if(batting_team == 1){
        printf("TAILS!\n\n");
    }
    sleep(1);
    //batting team A = 0, B = 1
    int teamAscore = 0;
    int teamBscore = 0;
    for(int i = 1; i < 10; i++){
        int inning_time = rand() % 4 + 1;
        int bscore_randomizer = rand() % 100 + 1;
        //probabilities (5% 3 runs, 10% 2 runs, 20% 1 run, 65% no run)

        //determining the inning
        if(i == 1){
            if(batting_team == 0){
                printf("[Baseball]: Its the start of the %dst and Team %s is up to bat\n", i, "A");
            }
            else if(batting_team == 1){
                printf("[Baseball]: Its the start of the %dst and Team %s is up to bat\n", i, "B");
            }
        }
        else if(i == 2){
            if(batting_team == 0){
                printf("[Baseball]: Its the start of the %dnd and Team %s is up to bat\n", i, "A");
            }
            else if(batting_team == 1){
                printf("[Baseball]: Its the start of the %dnd and Team %s is up to bat\n", i, "B");
            }
        }
        else if(i == 3){
            if(batting_team == 0){
                printf("[Baseball]: Its the start of the %drd and Team %s is up to bat\n", i, "A");
            }
            else if(batting_team == 1){
                printf("[Baseball]: Its the start of the %drd and Team %s is up to bat\n", i, "B");
            }
        }
        else if(i > 3){
            if(batting_team == 0){
                printf("[Baseball]: Its the start of the %dth and Team %s is up to bat\n", i, "A");
            }
            else if(batting_team == 1){
                printf("[Baseball]: Its the start of the %dth and Team %s is up to bat\n", i, "B");
            }
        }
        
        sleep(inning_time);

        //score handling
        if(bscore_randomizer >= 95){
            if(batting_team == 0){
                teamAscore += 3;
                printf("[Baseball]: With loaded bases team %s hit a grand slam!\n", "A");
                printf("[Baseball]: Total score for team %s: %d\n", "A", teamAscore);
            }
            else if(batting_team == 1){
                teamBscore += 3;
                printf("[Baseball]: With loaded bases team %s hit a grand slam!\n", "B");
                printf("[Baseball]: Total score for team %s: %d\n", "B", teamBscore);
            }
        }
        else if(bscore_randomizer < 95 && bscore_randomizer >= 85){
            if(batting_team == 0){
                teamAscore += 2;
                printf("[Baseball]: Team %s gets away with 2 runs\n", "A");
                printf("[Baseball]: Total score for team %s: %d\n", "A", teamAscore);
            }
            else if(batting_team == 1){
                teamBscore += 2;
                printf("[Baseball]: Team %s gets away with 2 runs\n", "B");
                printf("[Baseball]: Total score for team %s: %d\n", "B", teamBscore);
            }
        }
        else if(bscore_randomizer < 85 && bscore_randomizer >= 65){
            if(batting_team == 0){
                teamAscore += 1;
                printf("[Baseball]: Team %s only gets 1 runs\n", "A");
                printf("[Baseball]: Total score for team %s: %d\n", "A", teamAscore);
            }
            else if(batting_team == 1){
                teamBscore += 1;
                printf("[Baseball]: Team %s only gets 1 runs\n", "B");
                printf("[Baseball]: Total score for team %s: %d\n", "B", teamBscore);
            }
        }
        else if(bscore_randomizer < 65 && bscore_randomizer >= 1){
            if(batting_team == 0){
                printf("[Baseball]: Team %s has been SHUTOUT\n", "A");
                printf("[Baseball]: Total score for team %s: %d\n", "A", teamAscore);
            }
            else if(batting_team == 1){
                printf("[Baseball]: Team %s has been SHUTOUT\n", "B");
                printf("[Baseball]: Total score for team %s: %d\n", "B", teamBscore);
            }
        }
        
        //changing sides
        if(batting_team == 0){
            printf("[Baseball]: Team %s has finished thier turn with an inning time of %d0 minutes!\n\n", "A", inning_time);
            batting_team = 1;
        }
        else if(batting_team == 1){
            printf("[Baseball]: Team %s has finished thier turn with an inning time of %d0 minutes!\n\n", "B", inning_time);
            batting_team = 0;
        }
        sleep(1);
    }

    sleep(1);
    //Game finish
    if(teamAscore > teamBscore){
        printf("[Baseball]: With a score of %d to %d, Team %s is VICTORIOUS!\n\n", teamAscore, teamBscore, "A");
    }
    else if(teamBscore > teamAscore){
        printf("[Baseball]: With a score of %d to %d, Team %s is VICTORIOUS!\n\n", teamAscore, teamBscore, "B");
    }
    else if(teamAscore == teamBscore){
        printf("[Baseball]: With a score of %d to %d, There has been a tie. Booooo!\n\n", teamAscore, teamBscore);
    }
    sleep(1);

    //cleaning up players and data
    sem_wait(&athlete_semaphore);
    free(active_ids);
    for(int i = 0; i < MAX_BASEBALL_TEAM_COUNT; i++){
        //unpause players from both teams
        printf("[Baseball]: Thank you Player #%d from Team A, and Player #%d from Team B for playing!\n", teamA_ids[i], teamB_ids[i]);
        usleep(50000);
        pthread_cond_signal(&baseball_player_cond[teamA_ids[i] - 2001]);
        usleep(50000);
        pthread_cond_signal(&baseball_player_cond[teamB_ids[i] - 2001]);
        usleep(100000);
    }
    free(teamA_ids); free(teamB_ids);
    active_sport_completed = 1;
    sem_post(&athlete_semaphore);
    printf("[Stadium]: The Baseball Match is FINISHED! On to the next match\n\n");
    return NULL;
}
void *simulateSoccerGame(void* args){
    printf("\n[Stadium]: Starting the SOCCER GAME!\n");
    printf("[Soccer]: Generating \"balanced\" teams\n");
    
    int *teamA_ids = malloc(MAX_SOCCER_TEAM_COUNT * sizeof(int));
    int *teamB_ids = malloc(MAX_SOCCER_TEAM_COUNT * sizeof(int));

    sem_wait(&athlete_semaphore);
    active_ids = malloc(MAX_SOCCER_TEAM_COUNT * 2  * sizeof(int));
    printf("[Soccer]: Total number of players to choose from: %d\n", ready_soccer_players);
    int j = 0;
    int type = 0;
    int soccer_players_pool = ready_soccer_players;
    if((soccer_players_pool < (MAX_SOCCER_TEAM_COUNT * 2))){
        if(((soccer_players_pool % 2) != 0)){
            soccer_players_pool--;
        }
        for(int i = 0; i < (soccer_players_pool / 2); i++){
            int location = rand() % ready_soccer_players;
            for(int k = 0; k <= i; k++){
                if(teamA_ids[k] == soccer_player_ids[location] || teamB_ids[k] == soccer_player_ids[location]){
                    location = rand() % ready_soccer_players;
                    k = 0;
                }
            }
            
            sem_wait(&athlete_semaphore);
            active_ids[j] = soccer_player_ids[location] + 10000; j++;
            sem_post(&athlete_semaphore);
            
            teamA_ids[i] = soccer_player_ids[location];
            pthread_cond_signal(&soccer_player_cond[teamA_ids[i] - 1001]);
            printf("[Soccer]: Player #%d has joined team A!\n", teamA_ids[i]);
            usleep(50000);

            location = rand() % ready_soccer_players;
            for(int k = 0; k <= i; k++){
                if(teamB_ids[k] == soccer_player_ids[location] || teamA_ids[k] == soccer_player_ids[location]){
                    location = rand() % ready_soccer_players;
                    k = 0;
                }
            }
            sem_wait(&athlete_semaphore);
            active_ids[j] = soccer_player_ids[location] + 20000; j++;
            sem_post(&athlete_semaphore);
            
            teamB_ids[i] = soccer_player_ids[location];
            pthread_cond_signal(&soccer_player_cond[teamB_ids[i] - 1001]);
            
            printf("[Soccer]: Player #%d has joined team B!\n", teamB_ids[i]);
            usleep(50000); //between player asignments .05 seconds
        }
    }
    else if((soccer_players_pool >= (MAX_SOCCER_TEAM_COUNT * 2))){
        type = 1;
        for(int i = 0; i < MAX_SOCCER_TEAM_COUNT; i++){
            int location = rand() % ready_soccer_players;
            for(int k = 0; k <= i; k++){
                if(teamA_ids[k] == soccer_player_ids[location] || teamB_ids[k] == soccer_player_ids[location]){
                    location = rand() % ready_soccer_players;
                    k = 0;
                }
            }
            
            sem_wait(&athlete_semaphore);
            active_ids[j] = soccer_player_ids[location] + 10000; j++;
            sem_post(&athlete_semaphore);
            
            teamA_ids[i] = soccer_player_ids[location];
            pthread_cond_signal(&soccer_player_cond[teamA_ids[i] - 1001]);
            printf("[Soccer]: Player #%d has joined team A!\n", teamA_ids[i]);
            usleep(50000);

            location = rand() % ready_soccer_players;
            for(int k = 0; k <= i; k++){
                if(teamB_ids[k] == soccer_player_ids[location] || teamA_ids[k] == soccer_player_ids[location]){
                    location = rand() % ready_soccer_players;
                    k = 0;
                }
            }
            sem_wait(&athlete_semaphore);
            active_ids[j] = soccer_player_ids[location] + 20000; j++;
            sem_post(&athlete_semaphore);
            
            teamB_ids[i] = soccer_player_ids[location];
            pthread_cond_signal(&soccer_player_cond[teamB_ids[i] - 1001]);
            
            printf("[Soccer]: Player #%d has joined team B!\n", teamB_ids[i]);
            usleep(50000); //between player asignments .05 seconds
        }
    }
    sem_post(&athlete_semaphore);
    sleep(1);
    int playtime = rand() % 5 + 2;
    printf("[Soccer]: Playtime will be %d0 minutes!\n\n", playtime);
    
    sleep(playtime);
    //batting team A = 0, B = 1
    int teamAscore = rand() % 4;
    int teamBscore = rand() % 4;


    if(teamAscore > teamBscore){
        printf("[Soccer]: With a score of %d to %d, Team %s is VICTORIOUS!\n\n", teamAscore, teamBscore, "A");
    }
    else if(teamBscore > teamAscore){
        printf("[Soccer]: With a score of %d to %d, Team %s is VICTORIOUS!\n\n", teamAscore, teamBscore, "B");
    }
    else if(teamAscore == teamBscore){
        printf("[Soccer]: With a score of %d to %d, There has been a tie. Booooo!\n\n", teamAscore, teamBscore);
    }
    sleep(1); //buffer 1 second

    //cleaning up players and data
    sem_wait(&athlete_semaphore);
    free(active_ids);
    if(type == 0){
        for(int i = 0; i < (soccer_players_pool / 2); i++){
            //unpause players from both teams
            printf("[Soccer]: Thank you Player #%d from Team A, and Player #%d from Team B for playing!\n", teamA_ids[i], teamB_ids[i]);
            usleep(50000); //buffer time .05 seconds
            pthread_cond_signal(&soccer_player_cond[teamA_ids[i] - 1001]);
            usleep(50000);
            pthread_cond_signal(&soccer_player_cond[teamB_ids[i] - 1001]);
            usleep(100000);
        }
    }
    else if(type == 1){
        for(int i = 0; i < MAX_SOCCER_TEAM_COUNT; i++){
            //unpause players from both teams
            printf("[Soccer]: Thank you Player #%d from Team A, and Player #%d from Team B for playing!\n", teamA_ids[i], teamB_ids[i]);
            usleep(50000); //buffer time .05 seconds
            pthread_cond_signal(&soccer_player_cond[teamA_ids[i] - 1001]);
            usleep(50000);
            pthread_cond_signal(&soccer_player_cond[teamB_ids[i] - 1001]);
            usleep(100000);
        }
    }
    free(teamA_ids); free(teamB_ids);
    active_sport_completed = 1;
    sem_post(&athlete_semaphore);
    printf("[Stadium]: The Soccer Match is FINISHED! On to the next match\n\n");
    return NULL;
}
void *simulateFootballGame(void* args){
    printf("\n[Stadium]: Starting the FOOTBALL GAME!\n");
    printf("[Football]: Generating \"balanced\" teams\n");
    
    int *teamA_ids = malloc(MAX_FOOTBALL_TEAM_COUNT * sizeof(int));
    int *teamB_ids = malloc(MAX_FOOTBALL_TEAM_COUNT * sizeof(int));


    sem_wait(&athlete_semaphore);
    active_ids = malloc(MAX_FOOTBALL_TEAM_COUNT * 2  * sizeof(int));
    printf("[Football]: Total number of players to choose from: %d\n", ready_football_players);
    int j = 0;

    for(int i = 0; i < MAX_FOOTBALL_TEAM_COUNT; i++){
        int location = rand() % ready_football_players;
        for(int k = 0; k <= i; k++){
            if(teamA_ids[k] == football_player_ids[location] || teamB_ids[k] == football_player_ids[location]){
                location = rand() % ready_football_players;
                k = 0;
            }
        }
        
        sem_wait(&athlete_semaphore);
        active_ids[j] = football_player_ids[location] + 10000; j++;
        sem_post(&athlete_semaphore);
        
        teamA_ids[i] = football_player_ids[location];
        pthread_cond_signal(&football_player_cond[teamA_ids[i] - 3001]);
        printf("[Football]: Player #%d has joined team A!\n", teamA_ids[i]);

        location = rand() % ready_football_players;
        for(int k = 0; k <= i; k++){
            if(teamB_ids[k] == football_player_ids[location] || teamA_ids[k] == football_player_ids[location]){
                location = rand() % ready_football_players;
                k = 0;
            }
        }
        
        sem_wait(&athlete_semaphore);
        active_ids[j] = football_player_ids[location] + 20000; j++;
        sem_post(&athlete_semaphore);
        
        teamB_ids[i] = football_player_ids[location];
        pthread_cond_signal(&football_player_cond[teamB_ids[i] - 3001]);
        
        printf("[Football]: Player #%d has joined team B!\n", teamB_ids[i]);
        usleep(50000); //between team asignments .05 seconds
    }
    sem_post(&athlete_semaphore);
    sleep(1);
    printf("[Football]: Flipping a coin for team to start with offense. . . ");
    int offense_team = rand() % 2;
    if(offense_team == 0){
        printf("HEADS!\n\n");
    }
    else if(offense_team == 1){
        printf("TAILS!\n\n");
    }
    sleep(1);
    //batting team A = 0, B = 1
    int teamAscore = 0;
    int teamBscore = 0;

    for(int i = 1; i < 5; i++){
        int quarter_time = rand() % 4 + 1;
        int tscore_randomizer = rand() % 100 + 1;
        //probabilities: (%25 td, 15% kick, 10% int to nr, 5% int to kick, 5% int to td, 40% no change)

        //quarterly print out
        if(i == 1){
            if(offense_team == 0){
                printf("[Football]: Its the start of the %dst Quarter and Team %s is starting offense\n", i, "A");
            }
            else if(offense_team == 1){
                printf("[Football]: Its the start of the %dst Quarter and Team %s is starting offense\n", i, "B");
            }
        }
        else if(i == 2){
            if(offense_team == 0){
                printf("[Football]: Its the start of the %dnd Quarter and Team %s is starting offense\n", i, "A");
            }
            else if(offense_team == 1){
                printf("[Football]: Its the start of the %dnd Quarter and Team %s is starting offense\n", i, "B");
            }
        }
        else if(i == 3){
            if(offense_team == 0){
                printf("[Football]: Its the start of the %drd Quarter and Team %s is starting offense\n", i, "A");
            }
            else if(offense_team == 1){
                printf("[Football]: Its the start of the %drd Quarter and Team %s is starting offense\n", i, "B");
            }
        }
        else if(i > 3){
            if(offense_team == 0){
                printf("[Football]: Its the start of the %dth Quarter and Team %s is starting offense\n", i, "A");
            }
            else if(offense_team == 1){
                printf("[Football]: Its the start of the %dth Quarter and Team %s is starting offense\n", i, "B");
            }
        }

        sleep(quarter_time);

        if(tscore_randomizer >= 75){
            if(offense_team == 0){
                if(teamBscore == 0 && teamAscore == 0){
                    teamAscore += 7;
                }
                else if(teamBscore > 0 || teamAscore > 0){
                    teamAscore += 6;
                }
                printf("[Football]: Team %s has scored a TOUCHDOWN!\n", "A");
                printf("[Football]: Total score for team %s: %d\n", "A", teamAscore);
            }
            else if(offense_team == 1){
                if(teamBscore == 0 && teamAscore == 0){
                    teamBscore += 7;
                }
                else if(teamBscore > 0 || teamAscore > 0){
                    teamBscore += 6;
                }
                printf("[Football]: Team %s has scored a TOUCHDOWN!\n", "B");
                printf("[Football]: Total score for team %s: %d\n", "B", teamBscore);
            }
        }
        else if(tscore_randomizer < 75 && tscore_randomizer >= 60){
            if(offense_team == 0){
                teamAscore += 2;
                printf("[Football]: Team %s scored a KICK!\n", "A");
                printf("[Football]: Total score for team %s: %d\n", "A", teamAscore);
            }
            else if(offense_team == 1){
                teamBscore += 2;
                printf("[Football]: Team %s scored a KICK!\n", "B");
                printf("[Football]: Total score for team %s: %d\n", "B", teamBscore);
            }
        }
        else if(tscore_randomizer < 60 && tscore_randomizer >= 50){
            if(offense_team == 0){
                printf("[Football]: Team %s got intercepted but team %s did not score!\n", "A", "B");
                printf("[Football]: Total score for team %s: %d\n", "A", teamAscore);
            }
            else if(offense_team == 1){
                printf("[Football]: Team %s got intercepted but team %s did not score!\n", "B", "A");
                printf("[Football]: Total score for team %s: %d\n", "B", teamBscore);
            }
        }
        else if(tscore_randomizer < 50 && tscore_randomizer >= 45){
            if(offense_team == 0){
                teamBscore += 2;
                printf("[Football]: Team %s got intercepted and team %s scored a kick!\n", "A", "B");
                printf("[Football]: Total score for team %s: %d\n", "A", teamAscore);
            }
            else if(offense_team == 1){
                teamAscore += 2;
                printf("[Football]: Team %s got intercepted and team %s scored a kick!\n", "B", "A");
                printf("[Football]: Total score for team %s: %d\n", "B", teamBscore);
            }
        }
        else if(tscore_randomizer < 45 && tscore_randomizer >= 40){
            if(offense_team == 0){
                if(teamBscore == 0 && teamAscore == 0){
                    teamBscore += 7;
                }
                else if(teamBscore > 0 || teamAscore > 0){
                    teamBscore += 6;
                }
                printf("[Football]: Team %s got intercepted and team %s scored a TOUCHDOWN!\n", "A", "B");
                printf("[Football]: Total score for team %s: %d\n", "A", teamAscore);
            }
            else if(offense_team == 1){
                if(teamBscore == 0 && teamAscore == 0){
                    teamAscore += 7;
                }
                else if(teamBscore > 0 || teamAscore > 0){
                    teamAscore += 6;
                }
                printf("[Football]: Team %s got intercepted and team %s scored a TOUCHDOWN!\n", "B", "A");
                printf("[Football]: Total score for team %s: %d\n", "B", teamBscore);
            }
        }
        else if(tscore_randomizer < 40 && tscore_randomizer >= 1){
            if(offense_team == 0){
                printf("[Football]: Team %s could not score!\n", "A");
                printf("[Football]: Total score for team %s: %d\n", "A", teamAscore);
            }
            else if(offense_team == 1){
                printf("[Football]: Team %s could not score!\n", "B");
                printf("[Football]: Total score for team %s: %d\n", "B", teamBscore);
            }
        }

        if(offense_team == 0){
            printf("[Football]: Team %s has finished thier offensive with an quarter time of %d0 minutes!\n\n", "A", quarter_time);
            offense_team = 1;
        }
        else if(offense_team == 1){
            printf("[Football]: Team %s has finished thier turn with an inning time of %d0 minutes!\n\n", "B", quarter_time);
            offense_team = 0;
        }
        sleep(1);
    }

    if(teamAscore > teamBscore){
        printf("[Football]: With a score of %d to %d, Team %s is VICTORIOUS!\n\n", teamAscore, teamBscore, "A");
    }
    else if(teamBscore > teamAscore){
        printf("[Football]: With a score of %d to %d, Team %s is VICTORIOUS!\n\n", teamAscore, teamBscore, "B");
    }
    else if(teamAscore == teamBscore){
        printf("[Football]: With a score of %d to %d, There has been a tie. Booooo!\n\n", teamAscore, teamBscore);
    }
    sleep(1); //buffer 1 second


    //cleaning up players and data
    sem_wait(&athlete_semaphore);
    free(active_ids);
    for(int i = 0; i < MAX_FOOTBALL_TEAM_COUNT; i++){
        //unpause players from both teams
        printf("[Football]: Thank you Player #%d from Team A, and Player #%d from Team B for playing!\n", teamA_ids[i], teamB_ids[i]);
        usleep(50000); //buffer time .05 seconds
        pthread_cond_signal(&football_player_cond[teamA_ids[i] - 3001]);
        usleep(50000);
        pthread_cond_signal(&football_player_cond[teamB_ids[i] - 3001]);
        usleep(100000);
    }
    free(teamA_ids); free(teamB_ids);
    printf("[Stadium]: The Football Match is FINISHED! On to the next match\n\n");
    active_sport_completed = 1;
    sem_post(&athlete_semaphore);
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

    // sem_init(&athlete_semaphore, 0, 1);

    pthread_t* athletes = malloc(124 * sizeof(pthread_t));
    pthread_t sports_match;
    int counter = 0;

    int number_o_matches = 0;
    int max_number_o_matches = 4;
    printf("[Stadium]: We will host a total of %d different sports matches today!", max_number_o_matches);
    //insert a while true when done testing
    while(number_o_matches < max_number_o_matches){
        //player generation
        if((baseball_players + football_players + soccer_players) < (MAX_BASEBALL_PLAYERS + MAX_FOOTBALL_PLAYERS + MAX_SOCCER_PLAYERS)){
            pthread_create(&athletes[counter], NULL, generateAthlete, NULL);
            usleep(100000); //pausing .1 seconds between player generation to prevent overlap
            counter++;
        }

        //print the rejects and the new comers!
        if(active_sport_completed == 1){
            sem_wait(&athlete_semaphore);
            active_sport_completed = 0;
            printf("[Stadium]: %d New Comers, %d Players have been rejected.\n\n", ready_players_while_waiting, blocked_players_while_waiting);
            ready_players_while_waiting = 0;
            blocked_players_while_waiting = 0;
            active_sport = 0;
            active_sport_val = "";
            number_o_matches++;
            printf("[Stadium]: Total number of completed sports simulations: %d!\n", number_o_matches);
            sem_post(&athlete_semaphore);
            sleep(1);
            pthread_join(sports_match, NULL);
            if(number_o_matches == max_number_o_matches){
                printf("[Stadium]: That is all for today thank you everyone for coming!\n");
                exit(EXIT_SUCCESS);
            }
        }
        //Start football simulation
        if((ready_football_players > (MAX_FOOTBALL_TEAM_COUNT * 2)) && (prev_contained(3) == 0) && (active_sport == 0) && (active_sport_completed == 0)){
            sem_wait(&athlete_semaphore);
            active_sport = 1;
            active_sport_val = "FOOTBALL";
            previous_field[prev_field] = 3;
            prev_field++;
            if(prev_field > 1){
                prev_field = 0;
            }
            sem_post(&athlete_semaphore);
            printf("\n\n[Stadium]: Ready for a game of FOOTBALL?, Field Configuration: %s\n", active_sport_val);
            printf("[Stadium]: Make sure that there are no BASEBALL players or SOCCER players on the field\n");
            pthread_create(&sports_match, NULL, simulateFootballGame, NULL);
        }
        //start baseball simulation
        if((ready_baseball_players > (MAX_BASEBALL_TEAM_COUNT * 2)) && (prev_contained(2) == 0) && (active_sport == 0) && (active_sport_completed == 0)){
            sem_wait(&athlete_semaphore);
            active_sport = 1;
            active_sport_val = "BASEBALL";
            previous_field[prev_field] = 2;
            prev_field++;
            if(prev_field > 1){
                prev_field = 0;
            }
            sem_post(&athlete_semaphore);
            printf("\n\n[Stadium]: Ready for a game of BALL?, Field Configuration: %s\n", active_sport_val);
            printf("[Stadium]: Make sure that there are no SOCCER players or FOOTBALL players on the field\n");
            pthread_create(&sports_match, NULL, simulateBaseballGame, NULL);
        }
        //start soccer simulation
        else if((ready_soccer_players > MIN_SOCCER_TEAM_COUNT) && (prev_contained(1) == 0) && (active_sport == 0) && (active_sport_completed == 0)){
            sem_wait(&athlete_semaphore);
            active_sport = 1;
            active_sport_val = "SOCCER";
            previous_field[prev_field] = 1;
            prev_field++;
            if(prev_field > 1){
                prev_field = 0;
            }
            sem_post(&athlete_semaphore);
            printf("\n\n[Stadium]: Ready for a game of SOCCER?, Field Configuration: %s\n", active_sport_val);
            printf("[Stadium]: Make sure that there are no BASEBALL players or FOOTBALL players on the field\n");
            pthread_create(&sports_match, NULL, simulateSoccerGame, NULL);
        }
        usleep(75000); // .075 tick speed
    }


    for(int i = 0; i < counter; i++){
        printf("%d\n", pthread_join(athletes[i], NULL));
    }
    // sem_destroy(&athlete_semaphore);
    exit(0);
}