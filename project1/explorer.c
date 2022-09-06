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

    char *Directories[] = {"/home","/proc","/proc/sys","/usr","/boot","/sbin"};
    
    for(int i = 5;i>0; i--){
        int j = rand() % 6;
        char* tmp = Directories[j];
        Directories[j] = Directories[i];
        Directories[i] = tmp;
    }
 
    int rc;
    for(int i = 0; i < 6; i++){
        chdir(Directories[i]);
        printf("Selection #%d: %s\n", i + 1, Directories[i]);
        rc = fork();
        if(rc < 0){
            printf("Error forking\n");
            exit(EXIT_FAILURE);
        }   
        else if(rc == 0){
            char* cwd = NULL;
            getcwd(cwd, sizeof(cwd));
            char *command[] = {"ls", "-tr", NULL};
            printf("\t[Child #%d]: Executing \"%s %s\" command...\n", getpid(), command[0], command[1]);
            execvp("ls", command);
        }
        else{
            int status;
            if(waitpid(rc, &status, 0) == -1){
                exit(EXIT_FAILURE);
            }
            if(WIFEXITED(status)){
                int exit_code = WEXITSTATUS(status);
                printf("[Parent]: Child %d has finished with exit code, %d.\n", rc, exit_code);
            }
        }
    } 
    
   exit(0);
}
