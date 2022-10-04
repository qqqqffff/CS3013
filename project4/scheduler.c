#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

struct job {
    int id;
    int length;
    struct job *next;
};

enum scheduler_algorithm_type {FIFO, SJF, RR};

int exicution_time = 0;

int parseAlgorithm(char *v){
    if(v[0] == 'F' && v[1] == 'I' && v[2] == 'F' && v[3] == 'O'){
        printf("Exicution trace with FIFO\n");
        return FIFO;
    }
    else if(v[0] == 'S' && v[1] == 'J' && v[2] == 'F'){
        printf("Exicution trace with SJF\n");
        return SJF;
    }
    else if(v[0] == 'R' && v[1] == 'R'){
        printf("Exicution trace with RR\n");
        return RR;
    }
    printf("Invalid algorithm type\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv){
    //require argc 4
    if(argc != 4){
        printf("proper syntax: [method] [file] [exicution-time]\n");
        exit(EXIT_FAILURE);
    }

    int alg = parseAlgorithm(argv[1]);

    char *file_name = argv[2];
    FILE *in_file = fopen(file_name, "r");

    struct job *job_stack = malloc(sizeof(struct job));
    int numb_jobs = 0;
    char *buffer = malloc(sizeof(char));
    struct job parsed_job;
    while(fgets(buffer, sizeof(buffer), in_file) != NULL){
        parsed_job.length = atoi(buffer);
        job_stack[numb_jobs++] = parsed_job;
    }
    
    for(int i = 0; i < numb_jobs; i++){
        job_stack[i].id = i;
    }

    if(alg == FIFO){
        for(int i = 0; i < numb_jobs; i++){  
            printf("Job %d ran for: %d\n", job_stack[i].id, job_stack[i].length);
        }
        printf("End of Exicution FIFO\n");
    }
    else if(alg == SJF){
        for(int i = 0; i < numb_jobs; i++){
            for(int j = i + 1; j < numb_jobs; j++){
                if(job_stack[j].length < job_stack[i].length){
                    struct job temp = job_stack[i];
                    job_stack[i] = job_stack[j];
                    job_stack[j] = temp;
                }
            }
        }
        for(int i = 0; i < numb_jobs; i++){
            printf("Job %d ran for: %d\n", job_stack[i].id, job_stack[i].length);
        }
        printf("End of Exicution SJF\n");
    }
    else if(alg == RR){
        int rr_time = atoi(argv[3]);
        int completed = 0; int debug_counter = 0;
        while(!completed && debug_counter < 4){
            completed = 1;
            for(int i = 0; i < numb_jobs; i++){
                int runtime = 0;
                if(job_stack[i].length > rr_time){
                    runtime = rr_time;
                    job_stack[i].length -= rr_time;
                    printf("Job %d ran for: %d\n", job_stack[i].id, runtime);
                }
                else if(job_stack[i].length <= rr_time && job_stack[i].length != 0){
                    runtime = job_stack[i].length;
                    job_stack[i].length -= runtime;
                    printf("Job %d ran for: %d\n", job_stack[i].id, runtime);
                }
                
                if(job_stack[i].length != 0 && completed){
                    completed = 0;
                }
            }
        }
        printf("End of Exicution RR\n");
    }

    exit(EXIT_SUCCESS);
}