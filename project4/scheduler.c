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
enum execution_mode {EXECUTION, ANALYSIS, BOTH};

int exicution_time = 0;


//mode 0 is just algorithm exicution
//mode 1 is just analysis
//mode 2 is both

int parseAlgorithm(char *v, int scheduler_mode){
    if(v[0] == 'F' && v[1] == 'I' && v[2] == 'F' && v[3] == 'O'){
        if(scheduler_mode == EXECUTION || scheduler_mode == BOTH) printf("Execution trace with FIFO:\n");
        return FIFO;
    }
    else if(v[0] == 'S' && v[1] == 'J' && v[2] == 'F'){
        if(scheduler_mode == EXECUTION || scheduler_mode == BOTH) printf("Execution trace with SJF:\n");
        return SJF;
    }
    else if(v[0] == 'R' && v[1] == 'R'){
        if(scheduler_mode == EXECUTION || scheduler_mode == BOTH) printf("Execution trace with RR:\n");
        return RR;
    }
    printf("Invalid algorithm type\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv){
    //require argc 4
    if(argc != 4){
        printf("proper syntax: [method] [file] [execution-time]\n");
        exit(EXIT_FAILURE);
    }
    //setting the scheduler exicution mode
    int scheduler_mode = BOTH;
    int alg = parseAlgorithm(argv[1], scheduler_mode);

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
            job_stack[i].response = 0;
            job_stack[i].turnaround = 0;
            job_stack[i].wait = 0;
        }
        for(int i = 0; i < numb_jobs; i++){  
            if(scheduler_mode == EXECUTION || scheduler_mode == BOTH) printf("Job %d ran for: %d\n", job_stack[i].id, job_stack[i].length);
            //response time calculation
            if(i == 0){
                job_stack[i].response = 0;
            }
            else if(i != 0){
                job_stack[i].response = job_stack[i - 1].length + job_stack[i - 1].response;
            }

            //turnaround time calculation
            if(i == 0){
                job_stack[i].turnaround = job_stack[i].length;
            }
            else if(i != 0){
                job_stack[i].turnaround = job_stack[i - 1].turnaround + job_stack[i].length;
            }

            //wait time calculation
            job_stack[i].wait = job_stack[i].response;
        }
        if(scheduler_mode == EXECUTION || scheduler_mode == BOTH) printf("End of execution with FIFO.\n");
        if(scheduler_mode == ANALYSIS || scheduler_mode == BOTH) printf("Begin analyzing FIFO:\n");
        double avg_response = 0;
        double avg_turnaround = 0;
        double avg_wait = 0;
        for(int i = 0; i < numb_jobs; i++){
            avg_response += (double) job_stack[i].response;
            avg_turnaround += (double) job_stack[i].turnaround;
            avg_wait += (double) job_stack[i].wait;
            if(scheduler_mode == ANALYSIS || scheduler_mode == BOTH) printf("Job %d -- Response time: %d  Turnaround: %d  Wait: %d\n", job_stack[i].id, job_stack[i].response, job_stack[i].turnaround, job_stack[i].wait);
        }
        avg_response /= numb_jobs;
        avg_turnaround /= numb_jobs;
        avg_wait /= numb_jobs;
        if(scheduler_mode == ANALYSIS || scheduler_mode == BOTH) printf("Average -- Response: %.2f  Turnaround: %.2f  Wait: %.2f\n", avg_response, avg_turnaround, avg_wait);
        if(scheduler_mode == ANALYSIS || scheduler_mode == BOTH) printf("End analyzing FIFO.\n");
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
            job_stack[i].response = 0;
            job_stack[i].turnaround = 0;
            job_stack[i].wait = 0;
        }
        for(int i = 0; i < numb_jobs; i++){
            if(scheduler_mode == EXECUTION || scheduler_mode == BOTH) printf("Job %d ran for: %d\n", job_stack[i].id, job_stack[i].length);

            //response time calculation
            if(i == 0){
                job_stack[i].response = 0;
            }
            else if(i != 0){
                job_stack[i].response = job_stack[i - 1].length + job_stack[i - 1].response;
            }

            //turnaround time calculation
            if(i == 0){
                job_stack[i].turnaround = job_stack[i].length;
            }
            else if(i != 0){
                job_stack[i].turnaround = job_stack[i - 1].turnaround + job_stack[i].length;
            }

            //wait time calculation
            job_stack[i].wait = job_stack[i].response;
        }
        if(scheduler_mode == EXECUTION || scheduler_mode == BOTH) printf("End of execution with SJF.\n");
        if(scheduler_mode == ANALYSIS || scheduler_mode == BOTH) printf("Begin analyzing SJF:\n");
        double avg_response = 0;
        double avg_turnaround = 0;
        double avg_wait = 0;
        for(int i = 0; i < numb_jobs; i++){
            avg_response += (double) job_stack[i].response;
            avg_turnaround += (double) job_stack[i].turnaround;
            avg_wait += (double) job_stack[i].wait;
            if(scheduler_mode == ANALYSIS || scheduler_mode == BOTH) printf("Job %d -- Response time: %d  Turnaround: %d  Wait: %d\n", job_stack[i].id, job_stack[i].response, job_stack[i].turnaround, job_stack[i].wait);
        }
        avg_response /= numb_jobs;
        avg_turnaround /= numb_jobs;
        avg_wait /= numb_jobs;
        if(scheduler_mode == ANALYSIS || scheduler_mode == BOTH) printf("Average -- Response: %.2f  Turnaround: %.2f  Wait: %.2f\n", avg_response, avg_turnaround, avg_wait);
        if(scheduler_mode == ANALYSIS || scheduler_mode == BOTH) printf("End analyzing SJF.\n");
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
            job_stack[i].wait = job_stack[i].length;
            job_stack[i].turnaround = 0;
        }
        while(!completed){
            completed = 1;
            for(int i = 0; i < numb_jobs; i++){
                //algorithm implementation
                job_stack[i].updated = 0;
                if(job_stack[i].length > rr_time){
                    job_stack[i].runtime = rr_time;
                    job_stack[i].length -= rr_time;
                    job_stack[i].updated = 1;
                    if(scheduler_mode == EXECUTION || scheduler_mode == BOTH) printf("Job %d ran for: %d\n", job_stack[i].id, job_stack[i].runtime);
                }
                else if(job_stack[i].length <= rr_time && job_stack[i].length != 0){
                    job_stack[i].runtime = job_stack[i].length;
                    job_stack[i].length -= job_stack[i].runtime;
                    job_stack[i].updated = 1;
                    if(scheduler_mode == EXECUTION || scheduler_mode == BOTH) printf("Job %d ran for: %d\n", job_stack[i].id, job_stack[i].runtime);
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
                    if(job_stack[i].length == 0){
                        job_stack[i].wait = job_stack[i].turnaround - job_stack[i].wait;
                    }
                }
                else if(i == 0 && !job_stack[i].complete){
                    int prev_found = 0;
                    for(int j = numb_jobs; j >= 0; j--){
                        if(job_stack[j].updated && !prev_found){
                            job_stack[i].turnaround = job_stack[j].turnaround + job_stack[i].runtime;
                            //wait time calculation
                            if(job_stack[i].length == 0){
                                job_stack[i].wait = job_stack[i].turnaround - job_stack[i].wait;
                            }
                            prev_found = 1;
                        }
                        if(prev_found){
                            break;
                        }
                    }
                }
                if(i != 0 && !job_stack[i].complete){
                    //get the last ran job's turnaround time
                    int prev_found = 0;
                    for(int j = i - 1; j >= 0; j--){
                        if(job_stack[j].updated && !prev_found){
                            job_stack[i].turnaround = job_stack[j].turnaround + job_stack[i].runtime;
                            //wait time calculation
                            if(job_stack[i].length == 0){
                                job_stack[i].wait = job_stack[i].turnaround - job_stack[i].wait;
                            }
                            prev_found = 1;
                        }
                        if(j - 1 < 0 && !prev_found){
                            j = numb_jobs;
                        }
                        if(prev_found){
                            break;
                        }
                    }
                }
                if(job_stack[i].length == 0){
                    job_stack[i].complete = 1;
                }

                
                //breaking the round robin loop
                if(job_stack[i].length != 0 && completed){
                    completed = 0;
                }
            }
            cycle++;
        }
        if(scheduler_mode == EXECUTION || scheduler_mode == BOTH) printf("End of execution with RR.\n");
        if(scheduler_mode == ANALYSIS || scheduler_mode == BOTH) printf("Begin analyzing RR:\n");
        double avg_response = 0;
        double avg_turnaround = 0;
        double avg_wait = 0;
        for(int i = 0; i < numb_jobs; i++){
            avg_response += (double) job_stack[i].response;
            avg_turnaround += (double) job_stack[i].turnaround;
            avg_wait += (double) job_stack[i].wait;
            if(scheduler_mode == ANALYSIS || scheduler_mode == BOTH) printf("Job %d -- Response time: %d  Turnaround: %d  Wait: %d\n", job_stack[i].id, job_stack[i].response, job_stack[i].turnaround, job_stack[i].wait);
        }
        avg_response /= numb_jobs;
        avg_turnaround /= numb_jobs;
        avg_wait /= numb_jobs;
        if(scheduler_mode == ANALYSIS || scheduler_mode == BOTH) printf("Average -- Response: %.2f  Turnaround: %.2f  Wait: %.2f\n", avg_response, avg_turnaround, avg_wait);
        if(scheduler_mode == ANALYSIS || scheduler_mode == BOTH) printf("End analyzing RR.\n");
    }

    exit(EXIT_SUCCESS);
}