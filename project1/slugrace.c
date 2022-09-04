#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

int main(){
    int rc = fork();
    if(rc == 0){
        for(int i = 1; i < 5; i++){
            char *slugnumb = malloc(sizeof(char) * (int)log10(i));
            sprintf(slugnumb, "%d", i);
            char **cmdv;
            char *cmd = "./slug";
            cmdv = calloc(3, sizeof(char *));
            cmdv[0] = "./slug";
            cmdv[1] = slugnumb;
            cmdv[2] = NULL;
            if(fork() == 0){
                execvp(cmd, cmdv);
            }else{
                int status;
                if(waitpid(rc, &status, 0) == -1){
                    exit(EXIT_FAILURE);
                }
                else{
                    int exit_code = WEXITSTATUS(status);
                    printf("[Parent #%d]: Child #%d finished with code %d\n", getpid(), rc, exit_code);
                    exit(0);
                }
            }
        }
    }else if(rc > 0){
        // struct timespec start, finish, delta;
        // clock_gettime(CLOCK_REALTIME, &start);

        // int status;
        // printf("[Parent #%d]: I am waiting for %d to finish...\n", rc, getpid());
        // if(waitpid(rc, &status, 0) == -1){
        //     exit(EXIT_FAILURE);
        // }
        // else{
        //     int exit_code = WEXITSTATUS(status);
        //     printf("[Parent #%d]: Child #%d finished with code %d\n", getpid(), rc, exit_code);
        //     exit(0);
        // }
    }else{
        printf("Error forking");
        exit(EXIT_FAILURE);
    }


    exit(0);
}