#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define SIZE 64
#define MAX_FRAME 4

enum instruction_type {map, store, load};

unsigned char physical_memory[SIZE];
int processes[SIZE];
int amt_process = 0;

int eviction_index = 0;

int *disk_slots;
int disk_slot_size = 0;
char **filenames;

struct instruction{
    int process_id;
    enum instruction_type instruction_type;
    int virtual_address;
    int value;
} instruction;

int parseInstruction(char *inst, int debug){
    printf("[Memory Manager] Read Instruction: %s", inst);
    char buffer[strlen(inst)]; int buffer_loc = 0; int i_val = 0;

    for(int j = 0; j < sizeof(buffer); j++){
        buffer[j] = 0;
    }
    for(int i = 0; i < strlen(inst); i++){
        if(inst[i] == ',' || inst[i] == '\n'){
            if(i_val == 0){
                instruction.process_id = atoi(buffer);
                if(debug)
                    printf("p_id: %d\n", instruction.process_id);
            }
            else if(i_val == 1){
                if(strcmp(buffer, "map") == 0){
                    instruction.instruction_type = map;
                    if(debug)
                        printf("type: %d\n", instruction.instruction_type);
                }
                else if(strcmp(buffer, "store") == 0){
                    instruction.instruction_type = store;
                    if(debug)
                        printf("type: %d\n", instruction.instruction_type);
                }
                else if(strcmp(buffer, "load") == 0){
                    instruction.instruction_type = load;
                    if(debug)
                        printf("type: %d\n", instruction.instruction_type);
                }
                else{
                    printf("invalid instruction type\n");
                    return -1;
                }
            }
            else if(i_val == 2){
                instruction.virtual_address = atoi(buffer);
                if(debug)
                    printf("address: %d\n", instruction.virtual_address);
            }
            else if(i_val == 3){
                instruction.value = atoi(buffer);
                if(debug)
                    printf("value: %d\n", instruction.value);
            }
            i_val++;
            for(int j = 0; j < sizeof(buffer); j++){
                buffer[j] = 0;
            }
            buffer_loc = 0;
            if(i_val == 4) break;
        }
        else{
            buffer[buffer_loc] = inst[i];
            buffer_loc++;
        }
    }
    return 0;
}
int findPageTable(int process_id){
    for(int i = 0; i < MAX_FRAME; i++){
        if(process_id == physical_memory[(i * 16)] && amt_process >= 1) return 1;
    }
    return 0;
}
int findPageTableIndex(int process_id){
    for(int i = 0; i < MAX_FRAME; i++){
        if(process_id == physical_memory[i * 16]) return i;
    }
    return -1;
}
int isPageTable(int physical_frame_index){
    for(int i = 0; i < amt_process; i++){
        if(processes[i] == physical_memory[(16 * physical_frame_index)]) return 1;
    }
    return 0;
}
int searchOpening(int process_id){
    int frames[] = {-1, -1, -1, -1};
    for(int i = 0; i < MAX_FRAME; i++){
        //if there is a page table for the frame
        if(isPageTable(i)){
            // printf("found page table at %d\nusages: %d, %d, %d, %d\n", i, physical_memory[(i * 16) + 1], physical_memory[(i * 16) + 2], physical_memory[(i * 16) + 3], physical_memory[(i * 16) + 4]);
            //check which if there are frames that
            if(physical_memory[(i * 16) + 1] != 255){
                frames[0] = 1;
            }
            if(physical_memory[(i * 16) + 2] != 255){
                frames[1] = 1;
            }
            if(physical_memory[(i * 16) + 3] != 255){
                frames[2] = 1;
            }
            if(physical_memory[(i * 16) + 4] != 255){
                frames[3] = 1;
            }
        }
    }
    //if there is an unallocated frame return the unallocated frame
    for(int i = 0; i < MAX_FRAME; i++){
        if(frames[i] == -1) {
            return i;
        }
    }
    return -1;
}
int searchPTOpening(int process_id){
    int loc = findPageTableIndex(process_id);

    for(int i = 0; i < MAX_FRAME - 1; i++){
        if(physical_memory[(loc * 16) + (3 * i) + 5] == 255) return i;
    }
    //if there are too many entries then the page table will fill
    printf("Error: Page table is full");
    return -1;
}
int searchAllocation(int physical_frame_index){
    for(int i = 0; i < MAX_FRAME; i++){
        if(isPageTable(i)){
            //check which if there are frames that
            if(physical_memory[(i * 16) + physical_frame_index + 1] != 255){
                return physical_memory[(i * 16)]; //if exists then return the pid
            }
        }
    }
    return 255;
}
int clearPersistentData(int debug){
    /*
    * Helper function to clean out all the persistent data after exicution finishes
    */
    for(int i = 0; i < disk_slot_size; i++){
        if(debug) printf("removing %s: %d\n", filenames[i], remove(filenames[i]));
        else remove(filenames[i]);
    }
    return 0;
}

int * mm_swap(int process_id, int virtual_address, int value, int retrieve){
    /*
    * Swap function: swaps the frame at eviction index into memory
    * if retrieve is ticked retrieve the page otherwise only evict
    **/

    //retrieval of 1 means that the data is to be retrieved
    static int ocupancy[] = {-1, -1, -1, -1}; int occ_ind = 0;
    FILE *fp = NULL;
    if(retrieve){
        //if value is -1 that means its a temp swap just for the page table
        if(value == -1){
            //find the saved page table file
            for(int i = 0; i < disk_slot_size; i++){
                if(filenames[i][0] == 'p' && filenames[i][1] == 't'){
                    int disk_pid = -1; int c = 0; 
                    char *disk_pid_c = malloc(sizeof(char));
                    int s = strlen(filenames[0]);
                    for(int j = 0; j < s; j++){
                        char *temp = malloc(sizeof(char));
                        if(filenames[i][j] == '_'){
                            c++; j++;
                            if(c > 1) break;
                        }
                        if(c == 1){
                            sprintf(temp, "%c", filenames[i][j]);
                            strcat(disk_pid_c, temp);
                        }
                    }
                    disk_pid = atoi(disk_pid_c);

                    if(process_id == disk_pid){
                        fp = fopen(filenames[i], "r");
                        break;
                    }
                }
            }

            //reconstructing the page table from persistent data
            static int reconstructed_pt[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
            char *buffer = malloc(sizeof(char));
            fgets(buffer, 64, fp);
            int buffer_index = 0;
            for(int i = 0; i < 16; i++){
                char *temp_ind = malloc(sizeof(char)); char *temp_val = malloc(sizeof(char));
                for(;buffer_index < strlen(buffer); buffer_index++){
                    if(buffer[buffer_index] == ','){
                        buffer_index++;
                        break;
                    }
                    sprintf(temp_ind, "%c", buffer[buffer_index]);
                    strcat(temp_val, temp_ind);
                }
                reconstructed_pt[i] = atoi(temp_val);
            }
            fclose(fp);
            
            return reconstructed_pt;
        }
        else if(value == 0){
            //page table for the process doesnt exist in memory, search persistent data
            static int ret[] = {-1, -1};
            int *pt = mm_swap(process_id, virtual_address, -1, 1);
            int v_add = (int) floor(virtual_address / 16);
            for(int i = 0; i < MAX_FRAME - 1; i++){
                int pt_v_address = (int) floor(pt[3 * i + 7] / 10);
                if(pt_v_address == v_add){
                    //check if the frame is in storage
                    int *p_f = mm_swap(process_id, virtual_address, 1, 1);
                    if(pt[3 * i + 5]){
                        ret[0] = p_f[virtual_address % 16];
                    }
                    else if(!pt[3 * i + 5]){
                        int phy_loc = pt[3 * i + 7] % 10;
                        ret[0] = physical_memory[virtual_address % 16 + phy_loc * 16];
                        ret[1] = phy_loc;
                        return ret;
                    }

                    //find any other page table and rewrite ocupancy
                    int evic_pid = -1;
                    for(int j = 0; j < MAX_FRAME; j++){
                        if(isPageTable(j)){
                            if(occ_ind == 0) evic_pid = physical_memory[j * 16 + eviction_index + 1];
                            physical_memory[j * 16 + eviction_index + 1] = process_id;
                            if(occ_ind < 1){
                                for(int k = 0; k < MAX_FRAME; k++){
                                    ocupancy[k] = physical_memory[j * 16 + k + 1]; occ_ind++;
                                }
                            }
                        }
                    }
                    pt[eviction_index + 1] = process_id;
                    //clear out another frame
                    mm_swap(evic_pid, -1, -1, 0);
                    

                    //update occupancy
                    pt[3 * i + 7] = pt_v_address * 10 + eviction_index;
                    //todo rewrite
                    ret[1] = pt[3 * i + 7] % 10;
                    break;
                }
            }

            return ret;
        }
        else if(value == 1){
            //returning the frame from storage (deleted when done using) does not implement an array list to update the indecies of other frames
            int *f = malloc(sizeof(int));
            char *file_name = malloc(sizeof(char));
            for(int i = 0; i < disk_slot_size; i++){
                if(filenames[i][0] == 'f'){
                    int c = 0; int pid_v = -1;
                    char *pid_s = malloc(sizeof(char));
                    for(int j = 0; j < strlen(filenames[i]); j++){
                        char *temp = malloc(sizeof(char));
                        if(filenames[i][j] == '_'){
                            c++; j++;
                            if(c > 1) break;
                        }
                        if(c == 1){
                            sprintf(temp, "%c", filenames[i][j]);
                            strcat(pid_s, temp);
                        }
                    }
                    pid_v = atoi(pid_s);
                    if(pid_v == process_id){
                        file_name = filenames[i];
                        break;
                    }
                }
            }
            fp = fopen(file_name, "r");
            char *buffer = malloc(sizeof(char));
            fgets(buffer, 64, fp);
            int buffer_index = 0;
            for(int i = 0; i < 16; i++){
                char *temp_ind = malloc(sizeof(char)); char *temp_val = malloc(sizeof(char));
                for(;buffer_index < strlen(buffer); buffer_index++){
                    if(buffer[buffer_index] == ','){
                        buffer_index++;
                        break;
                    }
                    sprintf(temp_ind, "%c", buffer[buffer_index]);
                    strcat(temp_val, temp_ind);
                }
                f[i] = atoi(temp_val);
            }
            fclose(fp);
            return f;
        }
    }
    //retrieval of 0 means that data is to be stored
    else if(!retrieve){
        //check if the page to be evicted is a pagetable
        char *file_name = malloc(sizeof(char));
        if(isPageTable(eviction_index)){
            //initializing the file to write to
            int ev_pid = physical_memory[(eviction_index * 16)];
            sprintf(file_name, "pt_%d_%d.txt", ev_pid, disk_slot_size);
            fp = fopen(file_name, "w");
            filenames[disk_slot_size] = file_name;

            //adjusting the physical memory
            char *data = malloc(sizeof(char));
            for(int i = 0; i < 16; i++){
                char *temp = malloc(sizeof(char));
                if(i >= 1 && i < 5){
                    if((i - 1) != eviction_index) ocupancy[occ_ind++] = physical_memory[(eviction_index * 16) + i];
                    else if((i - 1) == eviction_index) ocupancy[occ_ind++] = process_id;
                }

                if(i != 15){
                    sprintf(temp, "%d,", physical_memory[(eviction_index * 16) + i]);
                    if(i != 0) physical_memory[(eviction_index * 16) + i] = 0;
                    else if(i == 0) physical_memory[(eviction_index * 16) + i] = 255;
                }
                else if(i == 15){
                    sprintf(temp, "%d", physical_memory[(eviction_index * 16) + i]);
                    physical_memory[(eviction_index * 16) + i] = 0;
                }
                strcat(data, temp);
            }
            //writing to the file and closing
            fprintf(fp, "%s", data);
            fclose(fp);
        }
        else{
            //find the page table that belongs to the evicted table
            int loc = findPageTableIndex(process_id);
            int ev_pt_pid = physical_memory[(loc * 16) + eviction_index + 1];
            int ev_pt_loc = findPageTableIndex(ev_pt_pid);

            //if not found search persistent memory
            if(ev_pt_loc == -1 && loc != -1){
                //case: the evicted page table is not in memory but the calling process' page table is
                physical_memory[(loc * 16) + eviction_index + 1] = process_id;
                int *temp_pt = mm_swap(ev_pt_pid, -1, -1, 1);
                //adjust ocupancy
                for(int i = 0; i < MAX_FRAME; i++){
                    temp_pt[i + 1] = physical_memory[(loc * 16) + i + 1];
                }
                //adjust storage of data
                int ev_v_frame = -1;
                for(int i = 0; i < MAX_FRAME - 1; i++){
                    int p_frame = temp_pt[i + ((3 * i) + 7)] % 10;
                    if(p_frame == eviction_index){
                        temp_pt[i + ((3 * i) + 5)] = 1;
                        ev_v_frame = (int) floor(temp_pt[i + ((3 * i) + 7)] / 10);
                        break;
                    }
                }
                //re-write to file
                int fn_index;
                for(fn_index = 0; fn_index < disk_slot_size; fn_index++){
                    if(filenames[fn_index][0] == 'p' && filenames[fn_index][1] == 't'){
                        int c = 0; int file_pid_int = -1;
                        char *file_pid = malloc(sizeof(char)); char *temp = malloc(sizeof(char));
                        for(int i = 0; i < strlen(filenames[fn_index]); i++){
                            if(filenames[fn_index][i] == '_'){
                                c++; i++;
                                if(c > 1) break;
                            }
                            if(c == 1){
                                sprintf(temp, "%c", filenames[fn_index][i]);
                                strcat(file_pid, temp);
                            }
                        }
                        file_pid_int = atoi(file_pid);
                        if(file_pid_int == ev_pt_pid){
                            break;
                        }
                    }
                }
                file_name = filenames[fn_index];
                fp = fopen(file_name, "w");
                char *data = malloc(sizeof(char));
                for(int i = 0; i < 16; i++){
                    char *temp = malloc(sizeof(char));
                    if(i != 15) sprintf(temp, "%d,", temp_pt[i]);
                    else if(i == 15) sprintf(temp, "%d", temp_pt[i]);
                    strcat(data, temp);
                }
                fprintf(fp, "%s", data);
                fclose(fp);
                

                
                //clean out the physical memory
                free(data);
                data = malloc(sizeof(char)); 
                char *file_name_b = malloc(sizeof(char));
                //todo: fix brute forcing
                if(physical_memory[eviction_index * 16] == 255) physical_memory[eviction_index * 16] = 0;
                for(int i = 0; i < 16; i++){
                    char *temp = malloc(sizeof(char));
                    if(i != 15){
                        sprintf(temp, "%d,", physical_memory[(eviction_index * 16) + i]);
                        physical_memory[(eviction_index * 16) + i] = 0;
                    }
                    else if(i == 15){
                        sprintf(temp, "%d", physical_memory[(eviction_index * 16) + i]);
                        physical_memory[(eviction_index * 16) + i] = 0;
                    }
                    strcat(data, temp);
                }
                physical_memory[(eviction_index * 16)] = 255;
                
                sprintf(file_name_b, "f_%d_%d_%d.txt", ev_pt_pid, ev_v_frame, disk_slot_size);
                filenames[disk_slot_size] = file_name_b;
                
                fp = fopen(file_name_b, "w");
                fprintf(fp, "%s", data);
                fclose(fp);
            }
            else if(ev_pt_loc != -1 && loc != -1){
                //the page table of the process with the page getting evict exists in memory
                //and the page table for the calling process exists

                //setting the occupancy of the evicted page and the page table
                physical_memory[(ev_pt_loc * 16) + eviction_index + 1] = process_id;
                physical_memory[(loc * 16) + eviction_index + 1] = process_id;

                //updating the page table to show that it is in storage
                for(int i = 0; i < MAX_FRAME - 1; i++){
                    int p_frame = physical_memory[(ev_pt_loc * 16) + ((3 * i) + 7)] % 10;
                    if(p_frame == eviction_index){
                        physical_memory[(ev_pt_loc * 16) + ((3 * i) + 5)] = 1;
                        break;
                    }
                }
            }
            else if(ev_pt_loc != -1 && loc == -1){
                //todo: implement -> (kind of an issue)
                //case: means that the page table for process calling swap is not inside of memory currently
            }
            else if(ev_pt_loc == -1 && loc == -1){
                //todo: implement -> this is a big problem
                //case: both the evicted page table and the calling process' page table does not exist in memory
            }
        }
        printf("Swapped frame %d to disk at swap slot %d\n", eviction_index, disk_slot_size);
        disk_slot_size++; eviction_index++;
    }
    return ocupancy;
}
int mm_create_page_table(int process_id, int virtual_address, int value){
    /*
    * Helper function for the map function
    */

    //returns an opening index if there are none swap out and create a new one
    int p_table_frame = searchOpening(process_id);
    int *occupancy = NULL;
    if(p_table_frame == -1){
        //perform swap 
        //retrieval value of 0 means that the data is to be stored and not retrieved
        occupancy = mm_swap(process_id, virtual_address, value, 0);
        p_table_frame = eviction_index - 1;
    }
    //set the frame info for the frames
    processes[amt_process] = process_id;
    amt_process++;
    //put the process id at the beginning of the page table
    physical_memory[(p_table_frame * 16)] = process_id;
    //show which frames of memory that the table has control over will be stored as
    //a 0, 1, 2, or 3
    
    //inputing which physical frames are occupied by which process into the page table
    if(occupancy == NULL){
        for(int i = 0; i < MAX_FRAME; i++){
            physical_memory[(p_table_frame * 16) + i + 1] = 255;
        }
        for(int i = 0; i < MAX_FRAME; i++){
            if(p_table_frame == i){
                physical_memory[(p_table_frame * 16) + i + 1] = process_id;
            }
            //search other page tables and determine if they have other pages allocated
            else{
                //j is a frame and if that frame has been allocated by another page table it will return that page tables process id
                int process = searchAllocation(i);
                if(process > -1){
                    physical_memory[(p_table_frame * 16) + i + 1] = process;
                }
            }
        }
    }
    else if(occupancy != NULL){
        physical_memory[(p_table_frame * 16)] = process_id;
        physical_memory[(p_table_frame * 16) + 1] = occupancy[0];
        physical_memory[(p_table_frame * 16) + 2] = occupancy[1];
        physical_memory[(p_table_frame * 16) + 3] = occupancy[2];
        physical_memory[(p_table_frame * 16) + 4] = occupancy[3];
    }
    printf("Put page table for PID %d into physical frame %d\n", process_id, p_table_frame);
    //max_frame - 1 shows that there can only be a maximum of 3 frames and a page table in memory at a time
    for(int i = 0; i < MAX_FRAME - 1; i++){
        //255 displays unmapped
        physical_memory[(p_table_frame * 16) + ((3 * i) + 7)] = 255; //virtual address for the frame in physical memory
        physical_memory[(p_table_frame * 16) + ((3 * i) + 6)] = 255; //read write value
        physical_memory[(p_table_frame * 16) + ((3 * i) + 5)] = 255; //if the frame is in storage
    }
    return 0;
}
int mm_map(int process_id, int virtual_address, int value){
    /*
    * Map Function, Value 0 means only readable, Value 1 means read and write
    * 0 - 15 corresponds to virtual frame 0
    * 15 - 31 corresponds to virtual frame 1
    * 32 - 47 corresponds to virtual frame 2
    * 48 - 63 corresponds to virtual frame 3
    **/

    //determine if page table for pid already exists
    int flag = findPageTable(process_id);


    //if the flag is set check the frame to see if it is also created
    if(flag){
        //check to see if the page is already mapped and if so check the value if not map the page
        int v_page = (int) floor(virtual_address / 16);
        int p_frame; int loc = findPageTableIndex(process_id);

        for(p_frame = 0; p_frame < MAX_FRAME - 1; p_frame++){
            int t_v_page = (int) floor(physical_memory[(loc * 16) + ((3 * p_frame) + 7)] / 10);
            if(t_v_page == v_page){
                //v_page was found and overwriting priveldges or not
                int rw_bit = physical_memory[(loc * 16) + ((3 * p_frame) + 6)];
                if(rw_bit != value){
                    physical_memory[(loc * 16) + ((3 * p_frame) + 6)] = value;
                    p_frame = physical_memory[(loc * 16) + ((3 * p_frame) + 7)] % 10;
                    printf("Updating permision for virtual page %d (frame %d)\n", v_page, p_frame);
                }
                else{
                    printf("Error: virtual page %d is already mapped with rw_bit=%d\n", v_page, value);
                    return -1;
                }
                return 0;
            }
        }
        int phys_opening = searchOpening(process_id);
        if(phys_opening == -1){
            mm_swap(process_id, virtual_address, value, 0);
            phys_opening = eviction_index - 1;
        }
        int pt_opening = searchPTOpening(process_id);
        physical_memory[(loc * 16) + phys_opening + 1] = process_id; //shows that this process occupies the physical address stored here
        physical_memory[(loc * 16) + ((3 * pt_opening) + 5)] = 0; //shows that it is not in storage (yet)
        physical_memory[(loc * 16) + ((3 * pt_opening) + 6)] = value; // read-write value
        physical_memory[(loc * 16) + ((3 * pt_opening) + 7)] = v_page * 10 + phys_opening; // virtual addresses (multiplier)
        printf("Mapped virtual address %d (page %d) into physical frame %d\n", virtual_address, v_page, phys_opening);
        return 0;
    }

    //if the flag is not set create a page table and map for the process
    else if(!flag){

        //create page table frame
        mm_create_page_table(process_id, virtual_address, value);
        
        //map the virtual address
        mm_map(process_id, virtual_address, value);
    }
    return 0;
}
int mm_store(int process_id, int virtual_address, int value){
    /*
    * Store function, value from 0 - 255
    * goes to the virtual address and check if it has been allocated to proccess id
    * if not throw an error
    * then store the value at the offset of the virtual address
    **/


    //finding the page table for the process_id
    int flag = findPageTable(process_id);

    //if the page table for the process exists
    if(flag){
        int address_frame = (int) floor(virtual_address / 16);
        int p_table_index = findPageTableIndex(process_id);
        for(int i = 0; i < MAX_FRAME - 1; i++){
            //unmasking the virtual frame from the page table
            int v_frame = (int) floor(physical_memory[(p_table_index * 16) + ((3 * i) + 7)] / 10);
            
            if(v_frame == address_frame){
                // check if it is writeable to
                int rw_flag = physical_memory[(p_table_index * 16) + ((3 * i) + 6)];
                if(rw_flag == 1){
                    //unmasking the physical frame from the pagetable
                    int phys_frame = physical_memory[(p_table_index * 16) + ((3 * i) + 7)] % 10;
                    int phys_address = (16 * phys_frame) + (virtual_address % 16);
                    physical_memory[phys_address] = value;
                    
                    printf("Stored value %d at virtual address %d (physical address %d)\n", value, virtual_address, phys_address);
                    return 0;
                }
                else{
                    printf("Error: writes are not allowed to this page\n");
                    return -1;
                }
            }
        }
    }
    //the page table does not exist
    //todo: see if the page table exists in memory
    // else{
    //     printf("Error: Page Table not found\n");
    //     return -1;
    // }
    return 0;
}
int mm_load(int process_id, int virtual_address, int value){
    /*
    * Load function: load the from the memory for the specified process id
    * if the process id does not exist throw an error
    **/

    //finding the page table for the process_id
    int flag = findPageTable(process_id);

    //if the page table for the process exists
    if(flag){
        int address_frame = (int) floor(virtual_address / 16);
        int p_table_index = findPageTableIndex(process_id);
        for(int i = 0; i < MAX_FRAME - 1; i++){
            //unmasking the virtual frame from the page table
            int v_frame = (int) floor(physical_memory[(p_table_index * 16) + ((3 * i) + 7)] / 10);
            
            if(v_frame == address_frame){
                //unmasking the physical frame from the pagetable
                int phys_frame = physical_memory[(p_table_index * 16) + ((3 * i) + 7)] % 10;
                int phys_address = (16 * phys_frame) + (virtual_address % 16);
                int v = physical_memory[phys_address];
                
                printf("The value %d is at virtual address %d (physical address %d)\n", v, virtual_address, phys_address);
                return 0;
            }
        }
        //todo: implement ->
        //case: not found check persistent data just in case
    }
    //page table not found in memory
    else if(!flag){
        //search the persistent data
        int *ret = mm_swap(process_id, virtual_address, 0, 1);
        if(ret[0] != -1){
            printf("The value %d is at virtual address %d (physical address %d)\n", ret[0], virtual_address, ret[1] * 16 + virtual_address);
        }
        else if(ret[0] == -1){
            printf("Error: not found!\n");
        }
    }
    
    return 0;
}

int main(int argc, char *argv[]){
    //initializing the arrays
    disk_slots = malloc(sizeof(int));
    filenames = malloc(sizeof(char **));
    for(int i = 0; i < MAX_FRAME; i++){
        processes[i] = -1;
        physical_memory[i * 16] = 255;
    }

    //reading instructions from file
    FILE *fp;
    char *instruction_file = "instruction_set.bin";
    fp = fopen(instruction_file, "r");
    if(fp == NULL){
        printf("File: %s, failed to open", instruction_file);
        exit(EXIT_FAILURE);
    }

    //running the instructions in sequential order (max instruction length = 64 characters)
    char *buffer = malloc(64 * sizeof(char));
    while(fgets(buffer, sizeof(buffer) + 16, fp) != NULL){
        parseInstruction(buffer, 0);
        if(instruction.instruction_type == map){
            mm_map(instruction.process_id, instruction.virtual_address, instruction.value);
        }
        else if(instruction.instruction_type == store){
            mm_store(instruction.process_id, instruction.virtual_address, instruction.value);
        }
        else if(instruction.instruction_type == load){
            mm_load(instruction.process_id, instruction.virtual_address, instruction.value);
        }
        free(buffer); buffer = malloc(64 * sizeof(char));
    }
    free(buffer);
    fclose(fp);
    printf("\n");
    exit(0);
}