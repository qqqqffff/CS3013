#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

struct job {
    int id;
    int length;
    int response;
    int turnaround;
    int wait;
    int runtime;
    int complete;
    int updated;
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
        if(rr_time <= 0){
            printf("Error: Invlaid Time Argument\n");
            exit(EXIT_FAILURE);
        }
        int completed = 0; int cycle = 0;
        //initializing the jobs
        for(int i = 0; i < numb_jobs; i++){
            job_stack[i].complete = 0;
            job_stack[i].wait = 0;
            job_stack[i].turnaround = 0;
        }
        while(!completed && cycle < 4){
            completed = 1;
            for(int i = 0; i < numb_jobs; i++){
                //algorithm implementation
                job_stack[i].updated = 0;
                if(job_stack[i].length > rr_time){
                    job_stack[i].runtime = rr_time;
                    job_stack[i].length -= rr_time;
                    job_stack[i].updated = 1;
                    printf("Job %d ran for: %d\t", job_stack[i].id, job_stack[i].runtime);
                }
                else if(job_stack[i].length <= rr_time && job_stack[i].length != 0){
                    job_stack[i].runtime = job_stack[i].length;
                    job_stack[i].length -= job_stack[i].runtime;
                    job_stack[i].updated = 1;
                    printf("Job %d ran for: %d\t", job_stack[i].id, job_stack[i].runtime);
                }
                
                //analysis calculations
                //response time calculation
                if(cycle == 0 && i == 0){
                    job_stack[i].response = 0;
                }
                else if(cycle == 0 && i != 0){
                    job_stack[i].response = job_stack[i - 1].response + job_stack[i - 1].runtime;
                }

                //turnaround time calculation
                if(i == 0 && cycle == 0){
                    job_stack[i].turnaround = job_stack[i].runtime;
                    printf("\n");
                }
                else if(i == 0 && !job_stack[i].complete){
                    job_stack[i].turnaround = job_stack[numb_jobs].turnaround;
                }
                if(i != 0 && !job_stack[i].complete){
                    //this represents the current job runtime of tasks that arent at the top of the stack
                    for(int j = i - 1; j >= 0; j--){
                        if(job_stack[j].updated){
                            printf("last job: %d\n", j);
                            break;
                        }
                    }
                }
                if(job_stack[i].length == 0){
                    job_stack[i].complete = 1;
                }
                // printf("curr turnaround: %d\n", job_stack[i].turnaround);

                
                //wait time calculation
                if(job_stack[i].complete != 0 && i != 0 && job_stack[i].wait == 0){
                    job_stack[i].wait = job_stack[i - 1].turnaround;
                }
                else if(job_stack[i].complete && i == 0 && job_stack[i].wait == 0 && cycle == 0){
                    job_stack[i].wait = 0;
                }
                else if(job_stack[i].complete && i == 0 && job_stack[i].wait == 0 && cycle > 0){
                    job_stack[i].wait = job_stack[numb_jobs].turnaround;
                }

                
                //breaking the round robin loop
                if(job_stack[i].length != 0 && completed){
                    completed = 0;
                }
            }
            cycle++;
        }
        printf("End of Exicution RR\n");
        printf("Begin analyzing RR:\n");
        double avg_response = 0;
        double avg_turnaround = 0;
        double avg_wait = 0;
        for(int i = 0; i < numb_jobs; i++){
            avg_response += (double) job_stack[i].response;
            avg_turnaround += (double) job_stack[i].turnaround;
            avg_wait += (double) job_stack[i].wait;
            printf("Job %d -- Response Time: %d Turnaround: %d Wait: %d\n", job_stack[i].id, job_stack[i].response, job_stack[i].turnaround, job_stack[i].wait);
        }
        avg_response /= numb_jobs;
        avg_turnaround /= numb_jobs;
        avg_wait /= numb_jobs;
        printf("Average -- Response: %.2f Turnaround: %.2f Wait: %.2f\n", avg_response, avg_turnaround, avg_wait);
        printf("End analyzing RR\n");
    }

    exit(EXIT_SUCCESS);
}