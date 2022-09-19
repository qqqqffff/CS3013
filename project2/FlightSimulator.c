#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>
#include <string.h>

#define MAX_SMALL_PLANES 37
#define MAX_LARGE_PLANES 15
#define MAX_PLANES_AT_TERMINAL 6
#define RUNWAY_SECTIONS 6


int small_plane_count = 0;
int large_plane_count = 0;
int airplane_ids[MAX_SMALL_PLANES + MAX_LARGE_PLANES];

int runway_sections[RUNWAY_SECTIONS];
int runway_section_ids[RUNWAY_SECTIONS];
int takeoff_queue[MAX_SMALL_PLANES + MAX_LARGE_PLANES];
int takeoff_queue_size = 0;
int landing_queue[MAX_SMALL_PLANES + MAX_LARGE_PLANES];
int landing_queue_size = 0;

int planes_at_terminal = 0;
int terminal_slots[MAX_PLANES_AT_TERMINAL];
int terminal_slots_id[MAX_PLANES_AT_TERMINAL];

pthread_mutex_t small_plane_mutex[MAX_SMALL_PLANES];
pthread_mutex_t large_plane_mutex[MAX_LARGE_PLANES];
pthread_cond_t small_plane_cond[MAX_SMALL_PLANES];
pthread_cond_t large_plane_cond[MAX_LARGE_PLANES];

sem_t airport_semaphore;


void *generatePlane(void *args){
    //plane size 0 is large and 1 is small
    int plane_size = rand() % 2;
    int my_id = 0; int err1 = 0;
    char *plane_size_val = NULL;
    sem_wait(&airport_semaphore);
    while(1){
        if((plane_size == 1) && (small_plane_count < MAX_SMALL_PLANES)){
            my_id = small_plane_count + 1001;
            pthread_mutex_init(&small_plane_mutex[small_plane_count], 0);
            pthread_cond_init(&small_plane_cond[small_plane_count], 0);
            airplane_ids[small_plane_count] = my_id;
            plane_size_val = "Small";
            small_plane_count++;
            printf("[Airport]: %s plane has been generated, total number of %s planes: %d\n", plane_size_val, plane_size_val, small_plane_count);
            break;
        }
        else if((plane_size == 1) && (small_plane_count == MAX_SMALL_PLANES)){
            if(err1 == 0) {
                printf("[Airport]: Max small plane generation reached!\n");
                err1 = 1;
            }
            plane_size = 0;
        }
        else if((plane_size == 0) && (large_plane_count < MAX_LARGE_PLANES)){
            my_id = large_plane_count + 2001;
            pthread_mutex_init(&large_plane_mutex[large_plane_count], 0);
            pthread_cond_init(&large_plane_cond[large_plane_count], 0);
            plane_size_val = "Large";
            large_plane_count++;
            printf("[Airport]: %s plane has been generated, total number of %s planes: %d\n", plane_size_val, plane_size_val, small_plane_count);
            break;
        }
        else if((plane_size == 0) && (large_plane_count == MAX_LARGE_PLANES)){
            if(err1 == 0) {
                printf("[Airport]: Max large plane generation reached!\n");
                err1 = 1;
            }
            plane_size = 1;
        }
    }
    sem_post(&airport_semaphore);
    usleep(500000);
    //todo implement a queue
    int my_terminal = 0;
    for(int i = 0; i < MAX_PLANES_AT_TERMINAL; i++){
        if(terminal_slots[i] == 0){
            sem_wait(&airport_semaphore);
            terminal_slots[i] = 1; //signals occupied
            terminal_slots_id[i] = my_id;
            sem_post(&airport_semaphore);
            my_terminal = i + 1;
            break;
        }
    }
    printf("\t[%s Plane #%d]: I have arrived at airport terminal %d. . . preparing to board!\n", plane_size_val, my_id, my_terminal);
    int passenger_load_time = rand() % 4 + 1;
    sleep(passenger_load_time);
    printf("\t[%s Plane #%d]: All the passengers loaded in %d0 minutes. . . preparing to taxi!\n", plane_size_val, my_id, passenger_load_time);
    sem_wait(&airport_semaphore);
    planes_at_terminal--;
    terminal_slots[my_terminal - 1] = 0;
    terminal_slots_id[my_terminal - 1] = 0;
    sem_post(&airport_semaphore);
    int taxi_time = rand() % 3 + 1;
    sleep(taxi_time);
    printf("\t[%s Plane #%d]: Taxi complete in %d minutes. . . prepare for take off\n", plane_size_val, my_id, taxi_time);
    usleep(100000);

    if(plane_size == 1) {
        //find possible starting point
        int starting_location = 0;
        int ending_location = 0;
        if(takeoff_queue_size == 0){
            for(int i = 0; i < RUNWAY_SECTIONS; i++){
                if(runway_sections[i] == 0){
                    if(i == 0){
                        if(runway_sections[1] == 1 && runway_sections[3] == 1){
                            continue;
                        }
                        else if(runway_sections[1] == 0){
                            sem_wait(&airport_semaphore);
                            runway_sections[i] = 1;
                            runway_sections[1] = 1;
                            runway_section_ids[i] = my_id;
                            runway_section_ids[1] = my_id;
                            sem_post(&airport_semaphore);
                            ending_location = 2;
                            starting_location = i + 1;
                            printf("\t[%s Plane #%d]: I will using takeoff path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                            break;
                        }
                        else if(runway_sections[3] == 0){
                            sem_wait(&airport_semaphore);
                            runway_sections[i] = 1;
                            runway_sections[3] = 1;
                            runway_section_ids[i] = my_id;
                            runway_section_ids[3] = my_id;
                            sem_post(&airport_semaphore);
                            ending_location = 4;
                            starting_location = i + 1;
                            printf("\t[%s Plane #%d]: I will using takeoff path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                            break;
                        }
                    }
                    else if(i == 1){
                        if(runway_sections[2] == 1 && runway_sections[0] == 1){
                            continue;
                        }
                        else if(runway_sections[0] == 0){
                            sem_wait(&airport_semaphore);
                            runway_sections[i] = 1;
                            runway_sections[0] = 1;
                            runway_section_ids[i] = my_id;
                            runway_section_ids[0] = my_id;
                            sem_post(&airport_semaphore);
                            ending_location = 1;
                            starting_location = i + 1;
                            printf("\t[%s Plane #%d]: I will using takeoff path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                            break;
                        }
                        else if(runway_sections[2] == 1){
                            sem_wait(&airport_semaphore);
                            runway_sections[i] = 1;
                            runway_sections[2] = 1;
                            runway_section_ids[i] = my_id;
                            runway_section_ids[2] = my_id;
                            sem_post(&airport_semaphore);
                            ending_location = 3;
                            starting_location = i + 1;
                            printf("\t[%s Plane #%d]: I will using takeoff path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                            break;
                        }
                    }
                    else if(i == 2){
                        if(runway_sections[1] == 1 && runway_sections[3] == 1 && runway_sections[4] == 1){
                            continue;
                        }
                        else if(runway_sections[1] == 0){
                            sem_wait(&airport_semaphore);
                            runway_sections[i] = 1;
                            runway_sections[1] = 1;
                            runway_section_ids[i] = my_id;
                            runway_section_ids[1] = my_id;
                            sem_post(&airport_semaphore);
                            ending_location = 2;
                            starting_location = i + 1;
                            printf("\t[%s Plane #%d]: I will using takeoff path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                            break;
                        }
                        else if(runway_sections[3] == 0){
                            sem_wait(&airport_semaphore);
                            runway_sections[i] = 1;
                            runway_sections[3] = 1;
                            runway_section_ids[i] = my_id;
                            runway_section_ids[3] = my_id;
                            sem_post(&airport_semaphore);
                            ending_location = 4;
                            starting_location = i + 1;
                            printf("\t[%s Plane #%d]: I will using takeoff path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                            break;
                        }
                        else if(runway_sections[4] == 0){
                            sem_wait(&airport_semaphore);
                            runway_sections[i] = 1;
                            runway_sections[4] = 1;
                            runway_section_ids[i] = my_id;
                            runway_section_ids[4] = my_id;
                            sem_post(&airport_semaphore);
                            ending_location = 5;
                            starting_location = i + 1;
                            printf("\t[%s Plane #%d]: I will using takeoff path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                            break;
                        }
                    }
                    else if(i == 3){
                        if(runway_sections[0] == 1 && runway_sections[2] == 1 && runway_sections[5] == 1){
                            continue;
                        }
                        else if(runway_sections[0] == 0){
                            sem_wait(&airport_semaphore);
                            runway_sections[i] = 1;
                            runway_sections[0] = 1;
                            runway_section_ids[i] = my_id;
                            runway_section_ids[0] = my_id;
                            sem_post(&airport_semaphore);
                            ending_location = 1;
                            starting_location = i + 1;
                            printf("\t[%s Plane #%d]: I will using takeoff path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                            break;
                        }
                        else if(runway_sections[2] == 0){
                            sem_wait(&airport_semaphore);
                            runway_sections[i] = 1;
                            runway_sections[2] = 1;
                            runway_section_ids[i] = my_id;
                            runway_section_ids[2] = my_id;
                            sem_post(&airport_semaphore);
                            ending_location = 3;
                            starting_location = i + 1;
                            printf("\t[%s Plane #%d]: I will using takeoff path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                            break;
                        }
                        else if(runway_sections[5] == 0){
                            sem_wait(&airport_semaphore);
                            runway_sections[i] = 1;
                            runway_sections[1] = 1;
                            runway_section_ids[i] = my_id;
                            runway_section_ids[1] = my_id;
                            sem_post(&airport_semaphore);
                            ending_location = 6;
                            starting_location = i + 1;
                            printf("\t[%s Plane #%d]: I will using takeoff path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                            break;
                        }
                    }
                    else if(i == 4){
                        if(runway_sections[2] == 1){
                            continue;
                        }
                        else if(runway_sections[2] == 0){
                            sem_wait(&airport_semaphore);
                            runway_sections[i] = 1;
                            runway_sections[2] = 1;
                            runway_section_ids[i] = my_id;
                            runway_section_ids[2] = my_id;
                            sem_post(&airport_semaphore);
                            ending_location = 3;
                            starting_location = i + 1;
                            printf("\t[%s Plane #%d]: I will using takeoff path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                            break;
                        }
                    }
                    else if(i == 5){
                        if(runway_sections[3] == 1){
                            continue;
                        }
                        else if(runway_sections[3] == 0){
                            sem_wait(&airport_semaphore);
                            runway_sections[i] = 1;
                            runway_sections[3] = 1;
                            runway_section_ids[i] = my_id;
                            runway_section_ids[3] = my_id;
                            sem_post(&airport_semaphore);
                            ending_location = 4;
                            starting_location = i + 1;
                            printf("\t[%s Plane #%d]: I will using takeoff path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                            break;
                        }
                    }
                }
            }
        }
        else if(starting_location == 0 || ending_location == 0 || takeoff_queue > 0){
                takeoff_queue[takeoff_queue_size] = my_id;
                takeoff_queue_size++;
                int no_takeoff = 1;
                while(no_takeoff){
                    printf("[Airport]: No suitable takeoff path found! %s Plane #%d has joined the takeoff queue (s: %d)\n", plane_size_val, my_id, takeoff_queue_size);
                    pthread_cond_wait(&small_plane_cond[my_id - 1001], &small_plane_mutex[my_id - 1001]);
                    //todo implement code to launch from queue when last plane goes
                    //find a suitable take off path when woken
                    for(int i = 0; i < RUNWAY_SECTIONS; i++){
                        if(runway_sections[i] == 0){
                            if(i == 0){
                                if(runway_sections[1] == 1 && runway_sections[3] == 1){
                                    continue;
                                }
                                else if(runway_sections[1] == 0){
                                    sem_wait(&airport_semaphore);
                                    runway_sections[i] = 1;
                                    runway_sections[1] = 1;
                                    runway_section_ids[i] = my_id;
                                    runway_section_ids[1] = my_id;
                                    sem_post(&airport_semaphore);
                                    ending_location = 2;
                                    starting_location = i + 1;
                                    printf("\t[%s Plane #%d]: Exiting queue, I will using takeoff path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                                    break;
                                }
                                else if(runway_sections[3] == 0){
                                    sem_wait(&airport_semaphore);
                                    runway_sections[i] = 1;
                                    runway_sections[3] = 1;
                                    runway_section_ids[i] = my_id;
                                    runway_section_ids[3] = my_id;
                                    sem_post(&airport_semaphore);
                                    ending_location = 4;
                                    starting_location = i + 1;
                                    printf("\t[%s Plane #%d]: Exiting queue, I will using takeoff path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                                    break;
                                }
                            }
                            else if(i == 1){
                                if(runway_sections[2] == 1 && runway_sections[0] == 1){
                                    continue;
                                }
                                else if(runway_sections[0] == 0){
                                    sem_wait(&airport_semaphore);
                                    runway_sections[i] = 1;
                                    runway_sections[0] = 1;
                                    runway_section_ids[i] = my_id;
                                    runway_section_ids[0] = my_id;
                                    sem_post(&airport_semaphore);
                                    ending_location = 1;
                                    starting_location = i + 1;
                                    printf("\t[%s Plane #%d]: Exiting queue, I will using takeoff path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                                    break;
                                }
                                else if(runway_sections[2] == 1){
                                    sem_wait(&airport_semaphore);
                                    runway_sections[i] = 1;
                                    runway_sections[2] = 1;
                                    runway_section_ids[i] = my_id;
                                    runway_section_ids[2] = my_id;
                                    sem_post(&airport_semaphore);
                                    ending_location = 3;
                                    starting_location = i + 1;
                                    printf("\t[%s Plane #%d]: Exiting queue, I will using takeoff path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                                    break;
                                }
                            }
                            else if(i == 2){
                                if(runway_sections[1] == 1 && runway_sections[3] == 1 && runway_sections[4] == 1){
                                    continue;
                                }
                                else if(runway_sections[1] == 0){
                                    sem_wait(&airport_semaphore);
                                    runway_sections[i] = 1;
                                    runway_sections[1] = 1;
                                    runway_section_ids[i] = my_id;
                                    runway_section_ids[1] = my_id;
                                    sem_post(&airport_semaphore);
                                    ending_location = 2;
                                    starting_location = i + 1;
                                    printf("\t[%s Plane #%d]: Exiting queue, I will using takeoff path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                                    break;
                                }
                                else if(runway_sections[3] == 0){
                                    sem_wait(&airport_semaphore);
                                    runway_sections[i] = 1;
                                    runway_sections[3] = 1;
                                    runway_section_ids[i] = my_id;
                                    runway_section_ids[3] = my_id;
                                    sem_post(&airport_semaphore);
                                    ending_location = 4;
                                    starting_location = i + 1;
                                    printf("\t[%s Plane #%d]: Exiting queue, I will using takeoff path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                                    break;
                                }
                                else if(runway_sections[4] == 0){
                                    sem_wait(&airport_semaphore);
                                    runway_sections[i] = 1;
                                    runway_sections[4] = 1;
                                    runway_section_ids[i] = my_id;
                                    runway_section_ids[4] = my_id;
                                    sem_post(&airport_semaphore);
                                    ending_location = 5;
                                    starting_location = i + 1;
                                    printf("\t[%s Plane #%d]: Exiting queue, I will using takeoff path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                                    break;
                                }
                            }
                            else if(i == 3){
                                if(runway_sections[0] == 1 && runway_sections[2] == 1 && runway_sections[5] == 1){
                                    continue;
                                }
                                else if(runway_sections[0] == 0){
                                    sem_wait(&airport_semaphore);
                                    runway_sections[i] = 1;
                                    runway_sections[0] = 1;
                                    runway_section_ids[i] = my_id;
                                    runway_section_ids[0] = my_id;
                                    sem_post(&airport_semaphore);
                                    ending_location = 1;
                                    starting_location = i + 1;
                                    printf("\t[%s Plane #%d]: Exiting queue, I will using takeoff path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                                    break;
                                }
                                else if(runway_sections[2] == 0){
                                    sem_wait(&airport_semaphore);
                                    runway_sections[i] = 1;
                                    runway_sections[2] = 1;
                                    runway_section_ids[i] = my_id;
                                    runway_section_ids[2] = my_id;
                                    sem_post(&airport_semaphore);
                                    ending_location = 3;
                                    starting_location = i + 1;
                                    printf("\t[%s Plane #%d]: Exiting queue, I will using takeoff path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                                    break;
                                }
                                else if(runway_sections[5] == 0){
                                    sem_wait(&airport_semaphore);
                                    runway_sections[i] = 1;
                                    runway_sections[1] = 1;
                                    runway_section_ids[i] = my_id;
                                    runway_section_ids[1] = my_id;
                                    sem_post(&airport_semaphore);
                                    ending_location = 6;
                                    starting_location = i + 1;
                                    printf("\t[%s Plane #%d]: Exiting queue, I will using takeoff path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                                    break;
                                }
                            }
                            else if(i == 4){
                                if(runway_sections[2] == 1){
                                    continue;
                                }
                                else if(runway_sections[2] == 0){
                                    sem_wait(&airport_semaphore);
                                    runway_sections[i] = 1;
                                    runway_sections[2] = 1;
                                    runway_section_ids[i] = my_id;
                                    runway_section_ids[2] = my_id;
                                    sem_post(&airport_semaphore);
                                    ending_location = 3;
                                    starting_location = i + 1;
                                    printf("\t[%s Plane #%d]: Exiting queue, I will using takeoff path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                                    break;
                                }
                            }
                            else if(i == 5){
                                if(runway_sections[3] == 1){
                                    continue;
                                }
                                else if(runway_sections[3] == 0){
                                    sem_wait(&airport_semaphore);
                                    runway_sections[i] = 1;
                                    runway_sections[3] = 1;
                                    runway_section_ids[i] = my_id;
                                    runway_section_ids[3] = my_id;
                                    sem_post(&airport_semaphore);
                                    ending_location = 4;
                                    starting_location = i + 1;
                                    printf("\t[%s Plane #%d]: Exiting queue, I will using takeoff path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                                    break;
                                }
                            }
                        }
                    }
                    if(starting_location != 0){
                        printf("[Airport]: %s Plane #%d is exiting takeoff queue!\n", plane_size_val, my_id);
                        //updating queue
                        sem_wait(&airport_semaphore);
                        takeoff_queue[0] = 0;
                        for(int i = 0; i < takeoff_queue_size - 1; i++){
                            takeoff_queue[i] = takeoff_queue[i + 1];
                        }
                        takeoff_queue_size--;
                        sem_post(&airport_semaphore);
                        break;
                    }
                    //if starting location is not updated then loop again (no queue changes)
                } 
        }       
        
        int takeoff_speed = rand() % 4 + 1;
        usleep(100000 * takeoff_speed);
        sem_wait(&airport_semaphore);
        printf("\t[%s Plane #%d]: Takeoff from zone %d to %d SUCCESSFUL! (t: %ds)\n", plane_size_val, my_id, starting_location, ending_location, takeoff_speed);
        sem_post(&airport_semaphore);

        //updating the runway sections
        sem_wait(&airport_semaphore);
        runway_section_ids[starting_location - 1] = 0;
        runway_sections[starting_location - 1] = 0;
        runway_section_ids[ending_location - 1] = 0;
        runway_sections[ending_location - 1] = 0;
        sem_post(&airport_semaphore);

        //priority (takeoff queue -> landing queue)
        if(takeoff_queue_size > 0){ //signaling the first plane in queue
            if(takeoff_queue[0] > 2000){
                printf("\t[%s Plane #%d]: Notifying Large Plane #%d of a runway opening!\n", plane_size_val, my_id, takeoff_queue[0]);
                pthread_cond_signal(&large_plane_cond[takeoff_queue[0] - 2001]); 
            }
            else if(takeoff_queue[0] < 2000){
                printf("\t[%s Plane #%d]: Notifying Small Plane #%d of a runway opening!\n", plane_size_val, my_id, takeoff_queue[0]);
                pthread_cond_signal(&small_plane_cond[takeoff_queue[0] - 1001]);
            }
            else{
                printf("error notifying!");
                exit(EXIT_FAILURE);
            }
        }
        else if(landing_queue_size > 0){
            if(landing_queue[0] > 2000){
                printf("\t[%s Plane #%d]: Notifying Large Plane #%d of a runway opening!\n", plane_size_val, my_id, landing_queue[0]);
                pthread_cond_signal(&large_plane_cond[landing_queue[0] - 2001]); 
            }
            else if(landing_queue[0] < 2000){
                printf("\t[%s Plane #%d]: Notifying Large Plane #%d of a runway opening!\n", plane_size_val, my_id, landing_queue[0]);
                pthread_cond_signal(&large_plane_cond[landing_queue[0] - 2001]); 
            }
            else{
                printf("error notifying!");
                exit(EXIT_FAILURE);
            }
        }
        //TODO figure out what to do while in air and landing protocall
        int flight_time = rand() % 4 + 2;
        sleep(flight_time);
        printf("\t[%s Plane #%d]: In the air for T:%d0 minutes! Attempting to land\n", plane_size_val, my_id, flight_time);

        //starting landing process TODO: Landing queue size
        starting_location = 0;
        ending_location = 0;
        if(landing_queue_size == 0){
            for(int i = 0; i < RUNWAY_SECTIONS; i++){
                if(runway_sections[i] == 0){
                    if(i == 0){
                        if(runway_sections[1] == 1 && runway_sections[3] == 1){
                            continue;
                        }
                        else if(runway_sections[1] == 0){
                            sem_wait(&airport_semaphore);
                            runway_sections[i] = 1;
                            runway_sections[1] = 1;
                            runway_section_ids[i] = my_id;
                            runway_section_ids[1] = my_id;
                            sem_post(&airport_semaphore);
                            ending_location = 2;
                            starting_location = i + 1;
                            printf("\t[%s Plane #%d]: I will using landing path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                            break;
                        }
                        else if(runway_sections[3] == 0){
                            sem_wait(&airport_semaphore);
                            runway_sections[i] = 1;
                            runway_sections[3] = 1;
                            runway_section_ids[i] = my_id;
                            runway_section_ids[3] = my_id;
                            sem_post(&airport_semaphore);
                            ending_location = 4;
                            starting_location = i + 1;
                            printf("\t[%s Plane #%d]: I will using landing path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                            break;
                        }
                    }
                    else if(i == 1){
                        if(runway_sections[2] == 1 && runway_sections[0] == 1){
                            continue;
                        }
                        else if(runway_sections[0] == 0){
                            sem_wait(&airport_semaphore);
                            runway_sections[i] = 1;
                            runway_sections[0] = 1;
                            runway_section_ids[i] = my_id;
                            runway_section_ids[0] = my_id;
                            sem_post(&airport_semaphore);
                            ending_location = 1;
                            starting_location = i + 1;
                            printf("\t[%s Plane #%d]: I will using landing path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                            break;
                        }
                        else if(runway_sections[2] == 1){
                            sem_wait(&airport_semaphore);
                            runway_sections[i] = 1;
                            runway_sections[2] = 1;
                            runway_section_ids[i] = my_id;
                            runway_section_ids[2] = my_id;
                            sem_post(&airport_semaphore);
                            ending_location = 3;
                            starting_location = i + 1;
                            printf("\t[%s Plane #%d]: I will using landing path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                            break;
                        }
                    }
                    else if(i == 2){
                        if(runway_sections[1] == 1 && runway_sections[3] == 1 && runway_sections[4] == 1){
                            continue;
                        }
                        else if(runway_sections[1] == 0){
                            sem_wait(&airport_semaphore);
                            runway_sections[i] = 1;
                            runway_sections[1] = 1;
                            runway_section_ids[i] = my_id;
                            runway_section_ids[1] = my_id;
                            sem_post(&airport_semaphore);
                            ending_location = 2;
                            starting_location = i + 1;
                            printf("\t[%s Plane #%d]: I will using landing path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                            break;
                        }
                        else if(runway_sections[3] == 0){
                            sem_wait(&airport_semaphore);
                            runway_sections[i] = 1;
                            runway_sections[3] = 1;
                            runway_section_ids[i] = my_id;
                            runway_section_ids[3] = my_id;
                            sem_post(&airport_semaphore);
                            ending_location = 4;
                            starting_location = i + 1;
                            printf("\t[%s Plane #%d]: I will using landing path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                            break;
                        }
                        else if(runway_sections[4] == 0){
                            sem_wait(&airport_semaphore);
                            runway_sections[i] = 1;
                            runway_sections[4] = 1;
                            runway_section_ids[i] = my_id;
                            runway_section_ids[4] = my_id;
                            sem_post(&airport_semaphore);
                            ending_location = 5;
                            starting_location = i + 1;
                            printf("\t[%s Plane #%d]: I will using landing path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                            break;
                        }
                    }
                    else if(i == 3){
                        if(runway_sections[0] == 1 && runway_sections[2] == 1 && runway_sections[5] == 1){
                            continue;
                        }
                        else if(runway_sections[0] == 0){
                            sem_wait(&airport_semaphore);
                            runway_sections[i] = 1;
                            runway_sections[0] = 1;
                            runway_section_ids[i] = my_id;
                            runway_section_ids[0] = my_id;
                            sem_post(&airport_semaphore);
                            ending_location = 1;
                            starting_location = i + 1;
                            printf("\t[%s Plane #%d]: I will using landing path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                            break;
                        }
                        else if(runway_sections[2] == 0){
                            sem_wait(&airport_semaphore);
                            runway_sections[i] = 1;
                            runway_sections[2] = 1;
                            runway_section_ids[i] = my_id;
                            runway_section_ids[2] = my_id;
                            sem_post(&airport_semaphore);
                            ending_location = 3;
                            starting_location = i + 1;
                            printf("\t[%s Plane #%d]: I will using landing path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                            break;
                        }
                        else if(runway_sections[5] == 0){
                            sem_wait(&airport_semaphore);
                            runway_sections[i] = 1;
                            runway_sections[1] = 1;
                            runway_section_ids[i] = my_id;
                            runway_section_ids[1] = my_id;
                            sem_post(&airport_semaphore);
                            ending_location = 6;
                            starting_location = i + 1;
                            printf("\t[%s Plane #%d]: I will using landing path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                            break;
                        }
                    }
                    else if(i == 4){
                        if(runway_sections[2] == 1){
                            continue;
                        }
                        else if(runway_sections[2] == 0){
                            sem_wait(&airport_semaphore);
                            runway_sections[i] = 1;
                            runway_sections[2] = 1;
                            runway_section_ids[i] = my_id;
                            runway_section_ids[2] = my_id;
                            sem_post(&airport_semaphore);
                            ending_location = 3;
                            starting_location = i + 1;
                            printf("\t[%s Plane #%d]: I will using landing path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                            break;
                        }
                    }
                    else if(i == 5){
                        if(runway_sections[3] == 1){
                            continue;
                        }
                        else if(runway_sections[3] == 0){
                            sem_wait(&airport_semaphore);
                            runway_sections[i] = 1;
                            runway_sections[3] = 1;
                            runway_section_ids[i] = my_id;
                            runway_section_ids[3] = my_id;
                            sem_post(&airport_semaphore);
                            ending_location = 4;
                            starting_location = i + 1;
                            printf("\t[%s Plane #%d]: I will using landing path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                            break;
                        }
                    }
                }
            }
        }
        else if(starting_location == 0 ||  ending_location == 0 || landing_queue_size > 0){
            landing_queue[landing_queue_size] = my_id;
            landing_queue_size++;
            int noland = 1;
            while(noland){
                printf("[Airport]: No suitable landing path found! %s Plane #%d has joined the landing queue (s: %d)\n", plane_size_val, my_id, landing_queue_size);
                pthread_cond_wait(&small_plane_cond[my_id - 2001], &small_plane_mutex[my_id - 2001]);
                //wait for opening
                for(int i = 0; i < RUNWAY_SECTIONS; i++){
                    if(runway_sections[i] == 0){
                        if(i == 0){
                            if(runway_sections[1] == 1 && runway_sections[3] == 1){
                                continue;
                            }
                            else if(runway_sections[1] == 0){
                                sem_wait(&airport_semaphore);
                                runway_sections[i] = 1;
                                runway_sections[1] = 1;
                                runway_section_ids[i] = my_id;
                                runway_section_ids[1] = my_id;
                                sem_post(&airport_semaphore);
                                ending_location = 2;
                                starting_location = i + 1;
                                printf("\t[%s Plane #%d]: I will be using landing path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                                break;
                            }
                            else if(runway_sections[3] == 0){
                                sem_wait(&airport_semaphore);
                                runway_sections[i] = 1;
                                runway_sections[3] = 1;
                                runway_section_ids[i] = my_id;
                                runway_section_ids[3] = my_id;
                                sem_post(&airport_semaphore);
                                ending_location = 4;
                                starting_location = i + 1;
                                printf("\t[%s Plane #%d]: I will be using landing path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                                break;
                            }
                        }
                        else if(i == 1){
                            if(runway_sections[2] == 1 && runway_sections[0] == 1){
                                continue;
                            }
                            else if(runway_sections[0] == 0){
                                sem_wait(&airport_semaphore);
                                runway_sections[i] = 1;
                                runway_sections[0] = 1;
                                runway_section_ids[i] = my_id;
                                runway_section_ids[0] = my_id;
                                sem_post(&airport_semaphore);
                                ending_location = 1;
                                starting_location = i + 1;
                                printf("\t[%s Plane #%d]: I will be using landing path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                                break;
                            }
                            else if(runway_sections[2] == 1){
                                sem_wait(&airport_semaphore);
                                runway_sections[i] = 1;
                                runway_sections[2] = 1;
                                runway_section_ids[i] = my_id;
                                runway_section_ids[2] = my_id;
                                sem_post(&airport_semaphore);
                                ending_location = 3;
                                starting_location = i + 1;
                                printf("\t[%s Plane #%d]: I will be using landing path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                                break;
                            }
                        }
                        else if(i == 2){
                            if(runway_sections[1] == 1 && runway_sections[3] == 1 && runway_sections[4] == 1){
                                continue;
                            }
                            else if(runway_sections[1] == 0){
                                sem_wait(&airport_semaphore);
                                runway_sections[i] = 1;
                                runway_sections[1] = 1;
                                runway_section_ids[i] = my_id;
                                runway_section_ids[1] = my_id;
                                sem_post(&airport_semaphore);
                                ending_location = 2;
                                starting_location = i + 1;
                                printf("\t[%s Plane #%d]: I will be using landing path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                                break;
                            }
                            else if(runway_sections[3] == 0){
                                sem_wait(&airport_semaphore);
                                runway_sections[i] = 1;
                                runway_sections[3] = 1;
                                runway_section_ids[i] = my_id;
                                runway_section_ids[3] = my_id;
                                sem_post(&airport_semaphore);
                                ending_location = 4;
                                starting_location = i + 1;
                                printf("\t[%s Plane #%d]: I will be using landing path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                                break;
                            }
                            else if(runway_sections[4] == 0){
                                sem_wait(&airport_semaphore);
                                runway_sections[i] = 1;
                                runway_sections[4] = 1;
                                runway_section_ids[i] = my_id;
                                runway_section_ids[4] = my_id;
                                sem_post(&airport_semaphore);
                                ending_location = 5;
                                starting_location = i + 1;
                                printf("\t[%s Plane #%d]: I will be using landing path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                                break;
                            }
                        }
                        else if(i == 3){
                            if(runway_sections[0] == 1 && runway_sections[2] == 1 && runway_sections[5] == 1){
                                continue;
                            }
                            else if(runway_sections[0] == 0){
                                sem_wait(&airport_semaphore);
                                runway_sections[i] = 1;
                                runway_sections[0] = 1;
                                runway_section_ids[i] = my_id;
                                runway_section_ids[0] = my_id;
                                sem_post(&airport_semaphore);
                                ending_location = 1;
                                starting_location = i + 1;
                                printf("\t[%s Plane #%d]: I will using be landing path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                                break;
                            }
                            else if(runway_sections[2] == 0){
                                sem_wait(&airport_semaphore);
                                runway_sections[i] = 1;
                                runway_sections[2] = 1;
                                runway_section_ids[i] = my_id;
                                runway_section_ids[2] = my_id;
                                sem_post(&airport_semaphore);
                                ending_location = 3;
                                starting_location = i + 1;
                                printf("\t[%s Plane #%d]: I will using be landing path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                                break;
                            }
                            else if(runway_sections[5] == 0){
                                sem_wait(&airport_semaphore);
                                runway_sections[i] = 1;
                                runway_sections[1] = 1;
                                runway_section_ids[i] = my_id;
                                runway_section_ids[1] = my_id;
                                sem_post(&airport_semaphore);
                                ending_location = 6;
                                starting_location = i + 1;
                                printf("\t[%s Plane #%d]: I will using be landing path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                                break;
                            }
                        }
                        else if(i == 4){
                            if(runway_sections[2] == 1){
                                continue;
                            }
                            else if(runway_sections[2] == 0){
                                sem_wait(&airport_semaphore);
                                runway_sections[i] = 1;
                                runway_sections[2] = 1;
                                runway_section_ids[i] = my_id;
                                runway_section_ids[2] = my_id;
                                sem_post(&airport_semaphore);
                                ending_location = 3;
                                starting_location = i + 1;
                                printf("\t[%s Plane #%d]: I will using be landing path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                                break;
                            }
                        }
                        else if(i == 5){
                            if(runway_sections[3] == 1){
                                continue;
                            }
                            else if(runway_sections[3] == 0){
                                sem_wait(&airport_semaphore);
                                runway_sections[i] = 1;
                                runway_sections[3] = 1;
                                runway_section_ids[i] = my_id;
                                runway_section_ids[3] = my_id;
                                sem_post(&airport_semaphore);
                                ending_location = 4;
                                starting_location = i + 1;
                                printf("\t[%s Plane #%d]: I will using be landing path %d to %d!\n", plane_size_val, my_id, starting_location, ending_location);
                                break;
                            }
                        }
                    }
                }
                if(starting_location != 0){
                    printf("[Airport]: %s Plane #%d is exiting landing queue!\n", plane_size_val, my_id);
                    //updating queue
                    sem_wait(&airport_semaphore);
                    takeoff_queue[0] = 0;
                    for(int i = 0; i < takeoff_queue_size - 1; i++){
                        takeoff_queue[i] = takeoff_queue[i + 1];
                    }
                    takeoff_queue_size--;
                    sem_post(&airport_semaphore);
                    break;
                }
            }
        }

        int landing_time = rand() % 4 + 1;
        usleep(100000 * landing_time);
        sem_wait(&airport_semaphore);
        printf("\t[%s Plane #%d]: Landed from zone %d to %d SUCCESSFUL! (t: %ds)\n", plane_size_val, my_id, starting_location, ending_location, landing_time);
        sem_post(&airport_semaphore);

        //updating the runway sections
        sem_wait(&airport_semaphore);
        runway_section_ids[starting_location - 1] = 0;
        runway_sections[starting_location - 1] = 0;
        runway_section_ids[ending_location - 1] = 0;
        runway_sections[ending_location - 1] = 0;
        sem_post(&airport_semaphore);

        if(takeoff_queue_size > 0){ //signaling the first plane in queue
            if(takeoff_queue[0] > 2000){
                printf("\t[%s Plane #%d]: Notifying Large Plane #%d of a runway opening!\n", plane_size_val, my_id, takeoff_queue[0]);
                pthread_cond_signal(&large_plane_cond[takeoff_queue[0] - 2001]); 
            }
            else if(takeoff_queue[0] < 2000){
                printf("\t[%s Plane #%d]: Notifying Small Plane #%d of a runway opening!\n", plane_size_val, my_id, takeoff_queue[0]);
                pthread_cond_signal(&small_plane_cond[takeoff_queue[0] - 1001]);
            }
            else{
                printf("error!");
            }
        }
        else if(landing_queue_size > 0){
            if(landing_queue[0] > 2000){
                printf("\t[%s Plane #%d]: Notifying Large Plane #%d of a runway opening!\n", plane_size_val, my_id, landing_queue[0]);
                pthread_cond_signal(&large_plane_cond[landing_queue[0] - 2001]); 
            }
            else if(landing_queue[0] < 2000){
                printf("\t[%s Plane #%d]: Notifying Small Plane #%d of a runway opening!\n", plane_size_val, my_id, landing_queue[0]);
                pthread_cond_signal(&small_plane_cond[landing_queue[0] - 1001]);
            }
            else{
                printf("error!");
            }
        }
    }
    else if(plane_size == 0){
        int starting_location = 0;
        int intermediate_location = 0;
        int ending_location = 0;
        if(takeoff_queue_size == 0){
            // -1 from max so that there are possible openings
            for(int i = 0; i < RUNWAY_SECTIONS; i++){
                if(runway_sections[i] == 0){
                    if(i == 0){
                        if(runway_sections[3] == 1 || runway_sections[5] == 1){
                            continue;
                        }
                        else if(runway_sections[3] == 0 && runway_sections[5] == 0){
                            sem_wait(&airport_semaphore);
                            runway_sections[i] = 1;
                            runway_sections[3] = 1;
                            runway_sections[5] = 1;
                            runway_section_ids[i] = my_id;
                            runway_section_ids[3] = my_id;
                            runway_section_ids[5] = my_id;
                            sem_post(&airport_semaphore);
                            starting_location = i + 1;
                            intermediate_location = 4;
                            ending_location = 6;
                            printf("\t[%s Plane #%d]: I will using takeoff path %d to %d to %d!\n", plane_size_val, my_id, starting_location, intermediate_location, ending_location);
                            break;
                        }
                    }
                    else if(i == 1){
                        if(runway_sections[2] == 1 || runway_sections[4] == 1){
                            continue;
                        }
                        else if(runway_sections[2] == 0 && runway_sections[4] == 0){
                            sem_wait(&airport_semaphore);
                            runway_sections[i] = 1;
                            runway_sections[2] = 1;
                            runway_sections[4] = 1;
                            runway_section_ids[i] = my_id;
                            runway_section_ids[2] = my_id;
                            runway_section_ids[4] = my_id;
                            sem_post(&airport_semaphore);
                            starting_location = i + 1;
                            intermediate_location = 3;
                            ending_location = 5;
                            printf("\t[%s Plane #%d]: I will using takeoff path %d to %d to %d!\n", plane_size_val, my_id, starting_location, intermediate_location, ending_location);
                            break;
                        }
                    }
                    else if(i == 4){
                        if(runway_sections[2] == 1 || runway_sections[1] == 1){
                            continue;
                        }
                        else if(runway_sections[2] == 0 && runway_sections[1] == 0){
                            sem_wait(&airport_semaphore);
                            runway_sections[i] = 1;
                            runway_sections[1] = 1;
                            runway_sections[2] = 1;
                            runway_section_ids[i] = my_id;
                            runway_section_ids[2] = my_id;
                            runway_section_ids[1] = my_id;
                            sem_post(&airport_semaphore);
                            starting_location = i + 1;
                            intermediate_location = 3;
                            ending_location = 2;
                            printf("\t[%s Plane #%d]: I will using takeoff path %d to %d to %d!\n", plane_size_val, my_id, starting_location, intermediate_location, ending_location);
                            break;
                        }
                    }
                    else if(i == 5){
                        if(runway_sections[3] == 1 || runway_sections[0] == 1){
                            continue;
                        }
                        else if(runway_sections[3] == 0){
                            sem_wait(&airport_semaphore);
                            runway_sections[i] = 1;
                            runway_sections[0] = 1;
                            runway_sections[3] = 1;
                            runway_section_ids[i] = my_id;
                            runway_section_ids[3] = my_id;
                            runway_section_ids[0] = my_id;
                            sem_post(&airport_semaphore);
                            starting_location = i + 1;
                            intermediate_location = 4;
                            ending_location = 1;
                            printf("\t[%s Plane #%d]: I will using takeoff path %d to %d to %d!\n", plane_size_val, my_id, starting_location, intermediate_location, ending_location);
                            break;
                        }
                    }
                }
            }
        }
        else if(starting_location == 0 || intermediate_location == 0 || ending_location == 0 || takeoff_queue_size > 0){
            takeoff_queue[takeoff_queue_size] = my_id;
            takeoff_queue_size++;
            int no_takeoff = 1;
            while(no_takeoff){
                printf("[Airport]: No suitable takeoff path found! %s Plane #%d has joined the takeoff queue (s: %d)\n", plane_size_val, my_id, takeoff_queue_size);
                pthread_cond_wait(&large_plane_cond[my_id - 1001], &large_plane_mutex[my_id - 1001]);
                //find a suitable take off path when woken
                for(int i = 0; i < RUNWAY_SECTIONS; i++){
                    if(runway_sections[i] == 0){
                        if(i == 0){
                            if(runway_sections[3] == 1 || runway_sections[5] == 1){
                                continue;
                            }
                            else if(runway_sections[3] == 0 && runway_sections[5] == 0){
                                sem_wait(&airport_semaphore);
                                runway_sections[i] = 1;
                                runway_sections[3] = 1;
                                runway_sections[5] = 1;
                                runway_section_ids[i] = my_id;
                                runway_section_ids[3] = my_id;
                                runway_section_ids[5] = my_id;
                                sem_post(&airport_semaphore);
                                starting_location = i + 1;
                                intermediate_location = 4;
                                ending_location = 6;
                                printf("\t[%s Plane #%d]: I will using takeoff path %d to %d to %d!\n", plane_size_val, my_id, starting_location, intermediate_location, ending_location);
                                break;
                            }
                        }
                        else if(i == 1){
                            if(runway_sections[2] == 1 || runway_sections[4] == 1){
                                continue;
                            }
                            else if(runway_sections[2] == 0 && runway_sections[4] == 0){
                                sem_wait(&airport_semaphore);
                                runway_sections[i] = 1;
                                runway_sections[2] = 1;
                                runway_sections[4] = 1;
                                runway_section_ids[i] = my_id;
                                runway_section_ids[2] = my_id;
                                runway_section_ids[4] = my_id;
                                sem_post(&airport_semaphore);
                                starting_location = i + 1;
                                intermediate_location = 3;
                                ending_location = 5;
                                printf("\t[%s Plane #%d]: I will using takeoff path %d to %d to %d!\n", plane_size_val, my_id, starting_location, intermediate_location, ending_location);
                                break;
                            }
                        }
                        else if(i == 4){
                            if(runway_sections[2] == 1 || runway_sections[1] == 1){
                                continue;
                            }
                            else if(runway_sections[2] == 0 && runway_sections[1] == 0){
                                sem_wait(&airport_semaphore);
                                runway_sections[i] = 1;
                                runway_sections[1] = 1;
                                runway_sections[2] = 1;
                                runway_section_ids[i] = my_id;
                                runway_section_ids[2] = my_id;
                                runway_section_ids[1] = my_id;
                                sem_post(&airport_semaphore);
                                starting_location = i + 1;
                                intermediate_location = 3;
                                ending_location = 2;
                                printf("\t[%s Plane #%d]: I will using takeoff path %d to %d to %d!\n", plane_size_val, my_id, starting_location, intermediate_location, ending_location);
                                break;
                            }
                        }
                        else if(i == 5){
                            if(runway_sections[3] == 1 || runway_sections[0] == 1){
                                continue;
                            }
                            else if(runway_sections[3] == 0){
                                sem_wait(&airport_semaphore);
                                runway_sections[i] = 1;
                                runway_sections[0] = 1;
                                runway_sections[3] = 1;
                                runway_section_ids[i] = my_id;
                                runway_section_ids[3] = my_id;
                                runway_section_ids[0] = my_id;
                                sem_post(&airport_semaphore);
                                starting_location = i + 1;
                                intermediate_location = 4;
                                ending_location = 1;
                                printf("\t[%s Plane #%d]: I will using takeoff path %d to %d to %d!\n", plane_size_val, my_id, starting_location, intermediate_location, ending_location);
                                break;
                            }
                        }
                    }
                }
                if(starting_location != 0){
                    printf("[Airport]: %s Plane #%d is exiting takeoff queue!\n", plane_size_val, my_id);
                    sem_wait(&airport_semaphore);
                    takeoff_queue[0] = 0;
                    for(int i = 0; i < takeoff_queue_size - 1; i++){
                        takeoff_queue[i] = takeoff_queue[i + 1];
                    }
                    takeoff_queue_size--;
                    sem_post(&airport_semaphore);
                    break;
                }
                //if starting location is not updated then loop again (no queue changes)
            }
        }
        
        int takeoff_speed = rand() % 4 + 1;
        usleep(100000 * takeoff_speed);
        sem_wait(&airport_semaphore);
        printf("\t[%s Plane #%d]: Takeoff from zone %d to %d to %d SUCCESSFUL! (t: %ds)\n", plane_size_val, my_id, starting_location, intermediate_location, ending_location, takeoff_speed);
        sem_post(&airport_semaphore);

        //updating the runway sections
        sem_wait(&airport_semaphore);
        runway_section_ids[starting_location - 1] = 0;
        runway_sections[starting_location - 1] = 0;
        runway_section_ids[intermediate_location - 1] = 0;
        runway_sections[intermediate_location - 1] = 0;
        runway_section_ids[ending_location - 1] = 0;
        runway_sections[ending_location - 1] = 0;
        sem_post(&airport_semaphore);

        if(takeoff_queue_size > 0){ //signaling the first plane in queue
            if(takeoff_queue[0] > 2000){
                printf("\t[%s Plane #%d]: Notifying Large Plane #%d of a runway opening!\n", plane_size_val, my_id, takeoff_queue[0]);
                pthread_cond_signal(&large_plane_cond[takeoff_queue[0] - 2001]); 
            }
            else if(takeoff_queue[0] < 2000){
                printf("\t[%s Plane #%d]: Notifying Small Plane #%d of a runway opening!\n", plane_size_val, my_id, takeoff_queue[0]);
                pthread_cond_signal(&small_plane_cond[takeoff_queue[0] - 1001]);
            }
            else{
                printf("error!");
            }
        }
        else if(landing_queue_size > 0){
            if(landing_queue[0] > 2000){
                printf("\t[%s Plane #%d]: Notifying Large Plane #%d of a runway opening!\n", plane_size_val, my_id, landing_queue[0]);
                pthread_cond_signal(&large_plane_cond[landing_queue[0] - 2001]); 
            }
            else if(landing_queue[0] < 2000){
                printf("\t[%s Plane #%d]: Notifying Large Plane #%d of a runway opening!\n", plane_size_val, my_id, landing_queue[0]);
                pthread_cond_signal(&large_plane_cond[landing_queue[0] - 2001]); 
            }
            else{
                printf("error notifying!");
                exit(EXIT_FAILURE);
            }
        }
        
        //total in air time
        int flight_time = rand() % 4 + 2;
        sleep(flight_time);
        printf("\t[%s Plane #%d]: In the air for T:%d0 minutes! Attempting to land\n", plane_size_val, my_id, flight_time);

        //starting landing process
        starting_location = 0;
        intermediate_location = 0;
        ending_location = 0;
        if(takeoff_queue_size == 0){
            for(int i = 0; i < RUNWAY_SECTIONS; i++){
            if(runway_sections[i] == 0){
                if(i == 0){
                    if(runway_sections[3] == 1 || runway_sections[5] == 1){
                        continue;
                    }
                    else if(runway_sections[3] == 0 && runway_sections[5] == 0){
                        sem_wait(&airport_semaphore);
                        runway_sections[i] = 1;
                        runway_sections[3] = 1;
                        runway_sections[5] = 1;
                        runway_section_ids[i] = my_id;
                        runway_section_ids[3] = my_id;
                        runway_section_ids[5] = my_id;
                        sem_post(&airport_semaphore);
                        starting_location = i + 1;
                        intermediate_location = 4;
                        ending_location = 6;
                        printf("\t[%s Plane #%d]: I will using landing path %d to %d to %d!\n", plane_size_val, my_id, starting_location, intermediate_location, ending_location);
                        break;
                    }
                }
                else if(i == 1){
                    if(runway_sections[2] == 1 || runway_sections[4] == 1){
                        continue;
                    }
                    else if(runway_sections[2] == 0 && runway_sections[4] == 0){
                        sem_wait(&airport_semaphore);
                        runway_sections[i] = 1;
                        runway_sections[2] = 1;
                        runway_sections[4] = 1;
                        runway_section_ids[i] = my_id;
                        runway_section_ids[2] = my_id;
                        runway_section_ids[4] = my_id;
                        sem_post(&airport_semaphore);
                        starting_location = i + 1;
                        intermediate_location = 3;
                        ending_location = 5;
                        printf("\t[%s Plane #%d]: I will using landing path %d to %d to %d!\n", plane_size_val, my_id, starting_location, intermediate_location, ending_location);
                        break;
                    }
                }
                else if(i == 4){
                    if(runway_sections[2] == 1 || runway_sections[1] == 1){
                        continue;
                    }
                    else if(runway_sections[2] == 0 && runway_sections[1] == 0){
                        sem_wait(&airport_semaphore);
                        runway_sections[i] = 1;
                        runway_sections[1] = 1;
                        runway_sections[2] = 1;
                        runway_section_ids[i] = my_id;
                        runway_section_ids[2] = my_id;
                        runway_section_ids[1] = my_id;
                        sem_post(&airport_semaphore);
                        starting_location = i + 1;
                        intermediate_location = 3;
                        ending_location = 2;
                        printf("\t[%s Plane #%d]: I will using landing path %d to %d to %d!\n", plane_size_val, my_id, starting_location, intermediate_location, ending_location);
                        break;
                    }
                }
                else if(i == 5){
                    if(runway_sections[3] == 1 || runway_sections[0] == 1){
                        continue;
                    }
                    else if(runway_sections[3] == 0){
                        sem_wait(&airport_semaphore);
                        runway_sections[i] = 1;
                        runway_sections[0] = 1;
                        runway_sections[3] = 1;
                        runway_section_ids[i] = my_id;
                        runway_section_ids[3] = my_id;
                        runway_section_ids[0] = my_id;
                        sem_post(&airport_semaphore);
                        starting_location = i + 1;
                        intermediate_location = 4;
                        ending_location = 1;
                        printf("\t[%s Plane #%d]: I will using landing path %d to %d to %d!\n", plane_size_val, my_id, starting_location, intermediate_location, ending_location);
                        break;
                    }
                }
            }
        }
        }
        else if(starting_location == 0 || intermediate_location == 0 || ending_location == 0 || takeoff_queue > 0){
            landing_queue[landing_queue_size] = my_id;
            landing_queue_size++;
            int noland = 1;
            while(noland){
                printf("[Airport]: No suitable landing path found! %s Plane #%d has joined the landing queue (s: %d)\n", plane_size_val, my_id, landing_queue_size);
                pthread_cond_wait(&large_plane_cond[my_id - 2001], &large_plane_mutex[my_id - 2001]);
                //wait for opening
                for(int i = 0; i < RUNWAY_SECTIONS; i++){
                    if(runway_sections[i] == 0){
                        if(i == 0){
                            if(runway_sections[3] == 1 || runway_sections[5] == 1){
                                continue;
                            }
                            else if(runway_sections[3] == 0 && runway_sections[5] == 0){
                                sem_wait(&airport_semaphore);
                                runway_sections[i] = 1;
                                runway_sections[3] = 1;
                                runway_sections[5] = 1;
                                runway_section_ids[i] = my_id;
                                runway_section_ids[3] = my_id;
                                runway_section_ids[5] = my_id;
                                sem_post(&airport_semaphore);
                                starting_location = i + 1;
                                intermediate_location = 4;
                                ending_location = 6;
                                printf("\t[%s Plane #%d]: I will using landing path %d to %d to %d!\n", plane_size_val, my_id, starting_location, intermediate_location, ending_location);
                                break;
                            }
                        }
                        else if(i == 1){
                            if(runway_sections[2] == 1 || runway_sections[4] == 1){
                                continue;
                            }
                            else if(runway_sections[2] == 0 && runway_sections[4] == 0){
                                sem_wait(&airport_semaphore);
                                runway_sections[i] = 1;
                                runway_sections[2] = 1;
                                runway_sections[4] = 1;
                                runway_section_ids[i] = my_id;
                                runway_section_ids[2] = my_id;
                                runway_section_ids[4] = my_id;
                                sem_post(&airport_semaphore);
                                starting_location = i + 1;
                                intermediate_location = 3;
                                ending_location = 5;
                                printf("\t[%s Plane #%d]: I will using landing path %d to %d to %d!\n", plane_size_val, my_id, starting_location, intermediate_location, ending_location);
                                break;
                            }
                        }
                        else if(i == 4){
                            if(runway_sections[2] == 1 || runway_sections[1] == 1){
                                continue;
                            }
                            else if(runway_sections[2] == 0 && runway_sections[1] == 0){
                                sem_wait(&airport_semaphore);
                                runway_sections[i] = 1;
                                runway_sections[1] = 1;
                                runway_sections[2] = 1;
                                runway_section_ids[i] = my_id;
                                runway_section_ids[2] = my_id;
                                runway_section_ids[1] = my_id;
                                sem_post(&airport_semaphore);
                                starting_location = i + 1;
                                intermediate_location = 3;
                                ending_location = 2;
                                printf("\t[%s Plane #%d]: I will using landing path %d to %d to %d!\n", plane_size_val, my_id, starting_location, intermediate_location, ending_location);
                                break;
                            }
                        }
                        else if(i == 5){
                            if(runway_sections[3] == 1 || runway_sections[0] == 1){
                                continue;
                            }
                            else if(runway_sections[3] == 0){
                                sem_wait(&airport_semaphore);
                                runway_sections[i] = 1;
                                runway_sections[0] = 1;
                                runway_sections[3] = 1;
                                runway_section_ids[i] = my_id;
                                runway_section_ids[3] = my_id;
                                runway_section_ids[0] = my_id;
                                sem_post(&airport_semaphore);
                                starting_location = i + 1;
                                intermediate_location = 4;
                                ending_location = 1;
                                printf("\t[%s Plane #%d]: I will using landing path %d to %d to %d!\n", plane_size_val, my_id, starting_location, intermediate_location, ending_location);
                                break;
                            }
                        }
                    }
                }
                if(starting_location != 0){
                    printf("[Airport]: %s Plane #%d is exiting landing queue!\n", plane_size_val, my_id);
                    //updating queue
                    sem_wait(&airport_semaphore);
                    takeoff_queue[0] = 0;
                    for(int i = 0; i < takeoff_queue_size - 1; i++){
                        takeoff_queue[i] = takeoff_queue[i + 1];
                    }
                    takeoff_queue_size--;
                    sem_post(&airport_semaphore);
                    break;
                }
            }
        }

        int landing_time = rand() % 4 + 1;
        usleep(100000 * landing_time);
        sem_wait(&airport_semaphore);
        printf("\t[%s Plane #%d]: Landed from zone %d to %d to %d SUCCESSFUL! (t: %ds)\n", plane_size_val, my_id, starting_location, intermediate_location, ending_location, landing_time);
        sem_post(&airport_semaphore);

        //updating the runway sections
        sem_wait(&airport_semaphore);
        runway_section_ids[starting_location - 1] = 0;
        runway_sections[starting_location - 1] = 0;
        runway_section_ids[intermediate_location - 1] = 0;
        runway_sections[intermediate_location - 1] = 0;
        runway_section_ids[ending_location - 1] = 0;
        runway_sections[ending_location - 1] = 0;
        sem_post(&airport_semaphore);

        if(takeoff_queue_size > 0){ //signaling the first plane in queue
            if(takeoff_queue[0] > 2000){
                printf("\t[%s Plane #%d]: Notifying Large Plane #%d of a runway opening!\n", plane_size_val, my_id, takeoff_queue[0]);
                pthread_cond_signal(&large_plane_cond[takeoff_queue[0] - 2001]); 
            }
            else if(takeoff_queue[0] < 2000){
                printf("\t[%s Plane #%d]: Notifying Small Plane #%d of a runway opening!\n", plane_size_val, my_id, takeoff_queue[0]);
                pthread_cond_signal(&small_plane_cond[takeoff_queue[0] - 1001]);
            }
            else{
                printf("error!");
            }
        }
        else if(landing_queue_size > 0){
            if(landing_queue[0] > 2000){
                printf("\t[%s Plane #%d]: Notifying Large Plane #%d of a runway opening!\n", plane_size_val, my_id, landing_queue[0]);
                pthread_cond_signal(&large_plane_cond[landing_queue[0] - 2001]); 
            }
            else if(landing_queue[0] < 2000){
                printf("\t[%s Plane #%d]: Notifying Large Plane #%d of a runway opening!\n", plane_size_val, my_id, landing_queue[0]);
                pthread_cond_signal(&large_plane_cond[landing_queue[0] - 2001]); 
            }
            else{
                printf("error notifying!");
                exit(EXIT_FAILURE);
            }
        }
    }
    taxi_time = rand() % 3 + 1;
    sleep(taxi_time);
    printf("\t[%s Plane #%d]: Taxi complete in %d minutes. . . prepare to unboard\n", plane_size_val, my_id, taxi_time);
    passenger_load_time = rand() % 4 + 1;
    sleep(passenger_load_time);
    printf("\t[%s Plane #%d]: All passengers unboarded in %d0 minutes!\n", plane_size_val, my_id, passenger_load_time);
    printf("[Airport]: %s Plane #%d has successfully complete its simulation!\n", plane_size_val, my_id);
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

    sem_init(&airport_semaphore, 0, 1);
    pthread_t* planes = malloc((MAX_SMALL_PLANES + MAX_SMALL_PLANES) * sizeof(pthread_t));

    printf("[Airport]: Lets start these flights!\n");
    for(int i = 0; i < (MAX_LARGE_PLANES + MAX_SMALL_PLANES); i++){
        if((small_plane_count + large_plane_count) < (MAX_SMALL_PLANES + MAX_LARGE_PLANES) && (planes_at_terminal < MAX_PLANES_AT_TERMINAL)){
            pthread_create(&planes[i], NULL, generatePlane, NULL);
            sem_wait(&airport_semaphore);
            planes_at_terminal++;
            sem_post(&airport_semaphore);
            usleep(500000); // .5 second generation interval
        }
        else if(planes_at_terminal == MAX_PLANES_AT_TERMINAL){
            printf("[Airport]: All the terminal slots have been FILLED!\n");
            printf("[Airport]: Waiting for opening\n");
            while(planes_at_terminal == MAX_PLANES_AT_TERMINAL){
                usleep(50000); //.05 second tick speed
            }
            printf("[Airport]: An opening has been discovered!\n");
        }
        else if((small_plane_count + large_plane_count) > (MAX_SMALL_PLANES + MAX_LARGE_PLANES)){
            break;
        }

    }

    for(int i = 0; i < (MAX_LARGE_PLANES + MAX_SMALL_PLANES); i++){
        pthread_join(planes[i], NULL);
    }
    sem_destroy(&airport_semaphore);
}

