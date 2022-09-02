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
    int children_count = (rand() % 6) + 8;
    printf("Number of decendents: %d\n", children_count);
    int rc = fork();
    while(children_count > 0){
        if(rc > 0){
            int status;
            printf("[Parent #%d]: I am waiting for %d to finish...\n", rc, getpid());
            if(waitpid(rc, &status, 0) == -1){
                exit(EXIT_FAILURE);
            }
            else{
                int exit_code = WEXITSTATUS(status);
                printf("[Parent #%d]: Child #%d finished with code %d\n", getpid(), rc, exit_code);
                exit(children_count);
            }
            
            
        }
        else if(rc == 0){
            printf("\t[Child #%d]: I am called with a count = %d\n", getpid(), children_count);
            children_count--;
            rc = fork();
        }
    }

    exit(0);
}