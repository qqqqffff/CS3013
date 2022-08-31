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
        exit(0);
    }

    int seed;
    fscanf(fp, "%d", &seed);
    printf("Read seed: %d\n", seed);
    srand(seed);

    int children = (rand() % 6) + 5;
    int *children_values = (int *) malloc(sizeof(children));
    int *children_pids = (int *) malloc(sizeof(children));
    
    for(int i = 0; i < children; i++){
        children_values[i] = (rand() % 256) + 1000;
        for(int j = 0; j < i; j++){
            if(children_values[i] == children_values[j]){
                children_values[i] = (rand() % 256) + 1000;
                j = 0;
            }
        }
    }
    for(int i = 0; i < children; i++){
        printf("children %d\'s number is: %d\n", i, children_values[i]);
    }

    int rc;
    if(rc < 0){
        printf("Error forking");
        exit(0);
    }
    rc = fork();
    for(int i = 0; i < children; i++){
        if(rc < 0){
            printf("Error forking");
            exit(0);
        }   
        else if(rc == 0){
            children = children_values[i];
            int wait_time = (children % 3) + 1;
            int exit_code = (children % 50) + 1;

            children_pids[i] = (int) getpid();
            printf("Hello, I am child: %d\ttime delayed: %d\treturn value: %d\n", children_pids[i], wait_time, exit_code);
            sleep(wait_time);
            exit(exit_code);
        }
        else{
            wait(NULL);
            rc = fork();
        }
    }

    exit(0);
}