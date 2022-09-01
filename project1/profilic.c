#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>


int main(){
    FILE *fp;
    fp = fopen("seed.txt", "r");

    if(fp == NULL){
        printf("Error reading seed.txt\n");
        exit(EXIT_FAILURE);
    }

    // seed generation
    int seed;
    fscanf(fp, "%d", &seed);
    printf("Read seed value: %d\n\n", seed);
    srand(seed);

    // creation of the array and the multiple children
    int children = (rand() % 6) + 5;
    int *children_values = (int *) malloc(sizeof(children));
    int *children_pids = (int *) malloc(sizeof(children));
    

    // assigning the random values
    for(int i = 0; i < children; i++){
        children_values[i] = (rand() % 256) + 1000;
        for(int j = 0; j < i; j++){
            if(children_values[i] == children_values[j]){
                children_values[i] = (rand() % 256) + 1000;
                j = 0;
            }
        }
    }

    // For debugging
    // for(int i = 0; i < children; i++){
    //     printf("children %d\'s number is: %d\n", i, children_values[i]);
    // }

    int rc = fork();
    if(rc < 0){
        printf("Error forking\n");
        exit(EXIT_FAILURE);
    }
    printf("[Parent]: Starting Child #%d\n", getpid());
    for(int i = 0; i < children; i++){
        if(rc < 0){
            printf("Error forking\n");
            exit(EXIT_FAILURE);
        }   
        else if(rc == 0){
            children = children_values[i];
            int wait_time = (children % 3) + 1;
            int exit_code = (children % 50) + 1;

            children_pids[i] = (int) getpid();
            printf("\t[Child #%d]:\ttime delayed: %d\texit code: %d\n", children_pids[i], wait_time, exit_code);
            sleep(wait_time);
            exit(exit_code);
        }
        else{
            int status;
            if(waitpid(rc, &status, 0) == -1){
                exit(EXIT_FAILURE);
            }
            if(WIFEXITED(status)){
                int exit_code = WEXITSTATUS(status);
                printf("[Parent]: Child %d has finished with exit code, %d.\n", getpid(), exit_code);
            }
            rc = fork();
        }
    }
    exit(0);
}