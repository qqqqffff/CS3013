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
   int i,j,tmp;
   for(i= 5;i>0; i--){
   	j=rand()%6;
   	tmp = Directories[j];
   	Directories[j] = Directories[i];
   	Directories[i] = tmp;
   }
    
   // printf("Directores: %d",sizeof(Directories));
    //char **Directories_order = calloc(6, sizeof(char *));

    // assigning the random values
    //printf("Assigning directory order");
    //for(int i = 0; i < 6; i++){
    //Directories_order[i] =  Directories[(rand() %6)+1];
  //  for(int j = 0; j<6; j++){
    //	    if(Directories[i] == Directories[j]){
      //          Directories_order[i] = Directories[(rand() % 6)+1];
        //        j = 0;
          //      }
         //   }
           // }
   // printf("finished assigning");
    // For debugging
    // for(int i = 0; i < children; i++){
    //     printf("children %d\'s number is: %d\n", i, children_values[i]);
    // }

    //int rc = fork();
   // if(rc < 0){
     //   printf("Error forking\n");
       // exit(EXIT_FAILURE);
    //}
    //if(rc > 0){
      //  printf("[Parent]: Starting Child #%d\n", rc);
    //}
    int rc;
    for(int i = 0; i < 6; i++){
    chdir(Directories[i]);
    printf("Directory: %d",Directories[i]);
    rc = fork();
        if(rc < 0){
            printf("Error forking\n");
            exit(EXIT_FAILURE);
        }   
        else if(rc == 0){
        char *cwd;
        getcwd(cwd,sizeof(cwd));
        char *command[] = {"ls ", "-tr",NULL};
        execvp("ls",command);
        exit(0);
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
