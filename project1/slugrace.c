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
    bool *completed[] = {false, false, false, false};
    int rc = fork();
    int child_amt = 2;
    if(rc == 0){
        for(int i = 1; i < child_amt; i++){
            char *slugnumb = malloc(sizeof(char) * (int)log10(i));
            sprintf(slugnumb, "%d", i);
            char **cmdv;
            char *cmd = "./slug";
            cmdv = calloc(3, sizeof(char *));
            cmdv[0] = "./slug";
            cmdv[1] = slugnumb;
            cmdv[2] = NULL;
            int rb = fork();
            if(rb > 0){
                int status;
                waitpid(rb, &status, 0);
            }
            else if(rb == 0){
                execvp(cmd, cmdv);
            }
            else{
                printf("Error forking");
                exit(EXIT_FAILURE);
            }
        }
    }
    else if(rc > 0){
        struct timespec start, finish, delta;
        clock_gettime(CLOCK_REALTIME, &start);
        int status;
        if(waitpid(rc, &status, 0) == -1){
            exit(EXIT_FAILURE);
        }
        else{
            clock_gettime(CLOCK_REALTIME, &finish);
            calc_delta(start, finish, &delta);
            printf("Time to completion: %d.%.9ld\n", (int)delta.tv_sec, delta.tv_nsec);
            
        }
        
    }else{
        printf("Error forking");
        exit(EXIT_FAILURE);
    }


    exit(0);
}