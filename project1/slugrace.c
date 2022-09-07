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
    int child_amt = 4;
    
    
    char **cmdv;
    cmdv = calloc(3, sizeof(char *));
    cmdv[0] = "./slug";
    cmdv[2] = NULL;
    int rc;
    clock_gettime(CLOCK_REALTIME, &start);
    printf("[Parent]: Starting the race\n");
    for(int i = 0; i < 4; i++){
        rc = fork();
        if(rc==0){
            completed[i] = getpid();
            char *slugnumb = malloc(sizeof(char) * (int)log10(child_amt));
            sprintf(slugnumb, "%d", i+1);
            cmdv[1] = slugnumb;
            execvp(cmdv[0], cmdv);
        }
        else{
            completed[i] = rc;
            usleep(1);
        }          
    }
    int cur_pid;
    int done = 1;
    while(done !=0){
        cur_pid = waitpid(rc, NULL, WNOHANG);
        clock_gettime(CLOCK_REALTIME, &finish);
        calc_delta(start, finish, &delta);
        if(cur_pid == 0){
            printf( "\n[Parent]: T=%d.%.9ld: ", (int)delta.tv_sec, delta.tv_nsec);
            for(int i = 0; i < 4; i++){
                if(completed[i] != 0){
                    printf( "%d \t ", completed[i]);
                }
                usleep(200000);
            }
        }
        else if(cur_pid > 0){
            printf("\n[Parent] Finished Race In: %d.%.9ld\n", (int)delta.tv_sec, delta.tv_nsec);
            for(int i = 0; i < 4; i++){
                if(completed[i] = cur_pid){
                    completed[i] = 0;
                }
            }
        }
        done = 0;
        for(int i = 0; i < 4; i++){
            if(completed[i] > done){
                done = completed[i];
            }
        }    
    }
    exit(0);
}
