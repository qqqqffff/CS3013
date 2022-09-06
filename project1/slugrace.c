#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <stdbool.h>

enum { ns_p_second = 1000000000};
void calc_delta(struct timespec t1, struct timespec t2, struct timespec *td){
    td->tv_nsec = t2.tv_nsec - t1.tv_nsec;
    td->tv_sec = t2.tv_sec - t1.tv_sec;
    if(td->tv_sec > 0 && td->tv_nsec < 0){
        td->tv_nsec += ns_p_second;
        td->tv_sec--;
    }else if(td->tv_sec < 0 && td->tv_nsec > 0){
        td->tv_nsec -= ns_p_second;
        td->tv_sec++;
    }
}
int main(){
    int completed[] = {0, 0, 0, 0};
    struct timespec start, finish, delta;
    clock_gettime(CLOCK_REALTIME, &start);
    printf("[Parent]: Starting the race\n");

    int rc = fork();
    int child_amt = 4;
    while(child_amt > 0){
        if(rc == 0){
            char *slugnumb = malloc(sizeof(char) * (int)log10(child_amt));
            sprintf(slugnumb, "%d", child_amt);
            char **cmdv;
            cmdv = calloc(3, sizeof(char *));
            cmdv[0] = "./slug";
            cmdv[1] = slugnumb;
            cmdv[2] = NULL;
            completed[4 - child_amt] = (int) getpid();
            if(fork() == 0){
                execvp(cmdv[0], cmdv);
            }
            else{

            }
            child_amt--;
            rc = fork();
        }
        else if(rc > 0){
            int status;
            waitpid(rc, &status, 0);
            // printf("child amt: %d\n", child_amt);
            exit(0);
        }
    }
    exit(0);
}

/*
if(rc == 0){
        int rb = fork();
        while(child_amt > 0){
            if(rb == 0){
                
            }
            else if(rb > 0){
                // double time = .2;
                // wait(&time);
                
                // printf("child finished\n");
                // while (waitpid(rb, &status, WNOHANG)==0){
                //     //printf("child finished\n");
                //     clock_gettime(CLOCK_REALTIME, &finish);
                //     calc_delta(start, finish, &delta);
                //     // printf("current slugs: %d", getpid());
                //     printf("Slug: %d  Time so far: %d.%.9ld\n", getpid(), (int)delta.tv_sec, delta.tv_nsec);
                //     usleep(200);
                // }
            }
            else{
                printf("Error forking\n");
                exit(EXIT_FAILURE);
            }

        }
    }
    else if(rc > 0){
        int status;
        if(waitpid(rc, &status, 0) == -1){
            exit(EXIT_FAILURE);
        }
        else{
            clock_gettime(CLOCK_REALTIME, &finish);
            calc_delta(start, finish, &delta);
            printf("Child: %d finished in: %d.%.9ld\n",rc, (int)delta.tv_sec, delta.tv_nsec);
        }    
    }
    else{
        printf("Error forking");
        exit(EXIT_FAILURE);
    }
*/