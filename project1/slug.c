#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

int main(int argc, char *argv[]){

    // argument testing
    //
    // printf("argc: %d\n", argc);
    // for(int i = 0; i < argc; i++){
    //     printf("argv[%d]: %s\n",i ,argv[i]);
    // }

    if(argc > 2){
        printf("More args than I can handle! [C = %d\n]", argc);
        exit(EXIT_FAILURE);
    }else if(argc == 1){
        printf("Need more args! [C = %d]\n", argc);
        exit(EXIT_FAILURE);
    }

    //seed creation
    FILE *fp;
    char seed_file[32] = "seed_slug_";
    strcat(strcat(seed_file, argv[1]), ".txt");
    if(atoi(argv[1]) > 4 || atoi(argv[1]) < 1){
        printf("%d, is not in the allowed range", atoi(argv[1]));
        exit(EXIT_FAILURE);
    }
    fp = fopen(seed_file, "r");
    if(fp == NULL){
        printf("File: %s, failed to open", seed_file);
        exit(EXIT_FAILURE);
    }
    int seed;
    fscanf(fp, "%d", &seed);
    // printf("[Slug #%d]: Read seed: %d, From %s\n\n", getpid(), seed , seed_file);
    srand(seed);

    char *cmd = NULL;
    char **cmdv;
    char *command = NULL;
    int time = (rand() % 5) + 4;
    int coinflip = rand() % 2;
    char *coinflip_n = NULL;
    if(coinflip == 1){
        cmd = "id";
        cmdv = calloc(3, sizeof(char *));
        cmdv[0] = "id";
        cmdv[1] = "-g";
        cmdv[2] = NULL;
        command = "id --group";
        coinflip_n = "heads";
    }
    else{
        cmd = "last";
        cmdv = calloc(4, sizeof(char *));
        cmdv[0] = "last";
        cmdv[1] = "-i";
        cmdv[2] = "-x";
        cmdv[3] = NULL;
        command = "last -i -x";
        coinflip_n = "tails";
    }
    printf("[Slug #%d]: I'll take %d seconds. Coin flip: %s\n", getpid(), time, coinflip_n);
    sleep(time);
    printf("[Slug #%d]: Breaktime over: executing \'%s\' command...\n", getpid(), command);
    if(fork() == 0){
        execvp(cmd, cmdv);
    }
    exit(0);
}