
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define FILENAME_LENGTH 100
#define MEMORY_STRATEGY_LENGTH 100
#define DIVISOR 100
#define MEMORY_CAPACITY 2048

typedef enum{
    READY,
    RUNNING,
    FINISHED
}ProcessState;

struct Process{
    char name[FILENAME_LENGTH];
    int arrival_time;
    int service_time;
    int memory_requirement;
    ProcessState state;
    struct Process *next;
    int frame[512];
    int num_page;
    int address;
};

struct PerformStat{
    char name[FILENAME_LENGTH];
    int arrival_time;
    int service_time;
    int completion_time;
    int turnaround_time;
    float overhead_time;
    struct PerformStat *next;
};

typedef struct {
    struct Process *head;
    struct Process *tail;
} ProcessQueue;

struct MemoryBlock {
    int is_allocated; 
    char process_name[FILENAME_LENGTH];
    int start_address;
    int length;
    struct MemoryBlock *next;
};


void get_input_line(int argc, char* argv[], char *filename, char* memory_strategy, int* quantum){
  //  *quantum = 0;
    for (int i = 1; i < argc; i+=2){
        // filename
        if (strcmp(argv[i], "-f") == 0 && i + 1 < argc){
            strcpy(filename, argv[i + 1]);
        }
        // memory_strategy
        else if (strcmp(argv[i], "-m") == 0 && i + 1 < argc){
            strcpy(memory_strategy, argv[i + 1]);
        }
        // quantum
        else if(strcmp(argv[i], "-q") == 0 && i + 1 < argc){
            *quantum = atoi(argv[i + 1]);
        }
    }
}

void add_process(struct Process **head, struct Process *new_process){
    if(*head == NULL){
        *head = new_process;
    }
    else{
        struct Process *current = *head;
        while(current->next != NULL){
            current = current -> next;
        }
        current->next = new_process;
    }
    new_process->next = NULL;

}

void add_stat(struct PerformStat **head, struct PerformStat *new_stat){
    if(*head == NULL){
        *head = new_stat;
    }
    else{
        struct PerformStat *current = *head;
        while(current->next != NULL){
            current = current -> next;
        }
        current->next = new_stat;
    }
    new_stat->next = NULL;

}
void update_stat_data(struct PerformStat **head, char* name, int completion_time){
    if(*head != NULL){
        struct PerformStat *current = *head;
        while(current != NULL){
            if (strcmp(name, current->name) == 0){
                current->completion_time = completion_time;
                current->turnaround_time = current->completion_time - current->arrival_time;
                current->overhead_time = (DIVISOR * current->turnaround_time) / (current->service_time);
            }current = current -> next;
            
        }
    }

}

// read input file and create a linked list of processes
int read_input_file(char *filename, struct Process **processes, struct PerformStat **statistics) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    int num_process = 0;
    int arrival_time, service_time, memory_requirement;
    char name[FILENAME_LENGTH];

    // Print the values read
    while (fscanf(file, "%d %s %d %d", &arrival_time, name, &service_time, &memory_requirement) == 4) {
        struct Process *new_process = malloc(sizeof(struct Process));
        struct PerformStat *new_statistics = malloc(sizeof(struct PerformStat));
        if (new_process == NULL) {
            fprintf(stderr, "Error: Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
        new_process->arrival_time = arrival_time;
        strcpy(new_process->name, name);
        new_process->service_time = service_time;
        new_process->memory_requirement = memory_requirement;
        new_process->next = NULL;
        add_process(processes, new_process);

        strcpy(new_statistics->name, name);
        new_statistics->arrival_time = arrival_time;
        new_statistics->service_time = service_time;
        new_statistics->next = NULL;
        add_stat(statistics, new_statistics);

        num_process++;
    }

    fclose(file);
    return num_process;
}

float mem_usage(struct MemoryBlock **memory){
    float using_mem = 0;
    float range_tail = 0;
    if(memory != NULL){
        struct MemoryBlock *head = *memory;
        while(head != NULL){
            if (head->next != NULL){
                range_tail = head->next->start_address;
            }else{
                range_tail = MEMORY_CAPACITY;
            }
            if (head->is_allocated == 1){
                using_mem  += range_tail -head->start_address;
            }
            head = head->next;
        }

    }
    return ceil(using_mem*DIVISOR / MEMORY_CAPACITY);
    
}

// Function to print all processes in the linked list
void print_processes(struct Process *head) {
    printf("Processes:\n");
    while (head != NULL) {
        printf("Arrival Time: %d, Name: %s, Service Time: %d, Memory Requirement: %d\n",
               head->arrival_time, head->name, head->service_time, head->memory_requirement);
        head = head->next;
    }
}

void print_memory(struct MemoryBlock *head) {
    printf("MemoryBlock:\n");
    while (head != NULL) {
        printf("is_allocated: %d, Name: %s, length: %d, start_address: %d\n",
               head->is_allocated, head->process_name, head->length, head->start_address);
        head = head->next;
    }
}

void aHead_to_bTail(struct Process **a, struct Process **b){
    if (*a != NULL ){
        struct Process* oldHeadA = *a;
        
        if(*b == NULL){
            *b = *a;  // If list b is empty, the head of a becomes the head of b
        }
        else {
            struct Process* tailB = *b;
            while (tailB->next != NULL){
                tailB = tailB->next;
            }
            tailB->next = *a;  // Append the head of a to the tail of b
        }
        
        *a = (*a)->next;  // Move to the next element in a
        oldHeadA->next = NULL;  // Set next of old head of a to NULL, as it's now the last element
    }
}

void sort_by_arrival_time(struct Process** head) {
    if (*head == NULL || (*head)->next == NULL) {
        // If the list is empty or has only one node, it's already sorted
        return;
    }

    struct Process* sorted = NULL; // Initialize sorted list
    struct Process* current = *head; // Take the current node

    // Traverse the original list
    while (current != NULL) {
        struct Process* next_node = current->next; // Store the next node

        // If the current node should be the new head of the sorted list
        if (sorted == NULL || current->arrival_time < sorted->arrival_time) {
            // (current->arrival_time == sorted->arrival_time && current->memory_requirement < sorted->memory_requirement)
            current->next = sorted; // Insert the current node at the beginning
            sorted = current; // Update the head of the sorted list
        } else {
            // Else, find the right position to insert the current node in the sorted list
            struct Process* traverse = sorted;
            while (traverse->next != NULL &&
                   (traverse->next->arrival_time < current->arrival_time ||
                    (traverse->next->arrival_time == current->arrival_time && traverse->next->memory_requirement < current->memory_requirement))) {
                traverse = traverse->next;
            }
            // Insert the current node after the traverse node
            current->next = traverse->next;
            traverse->next = current;
        }

        current = next_node; // Move to the next node
    }

    // Update the head pointer to point to the sorted list
    *head = sorted;
}

void remove_head(struct Process** head){
    if (*head != NULL){
        *head = (*head)->next;
    }
}

const char* stateToString(ProcessState state) {
    if (state == READY) {
        return "READY";
    } else if (state == RUNNING) {
        return "RUNNING";
    } else {
        return "FINISHED";
    } 
}

int list_length(struct Process* head) {
    int length = 0;
    struct Process* current = head; // Start from the head of the list

    // Traverse the list and count nodes
    while (current != NULL) {
        length++;
        current = current->next; // Move to the next node
    }

    return length;
}

int ready_process_num(struct PerformStat **stat, int time){
    int counter = 0;
    if (*stat!= NULL){
        struct PerformStat *head = *stat;
        while (head != NULL){
            if (head->arrival_time <= time){
                counter++;
            }
            head = head->next;
        }

    }
    return counter;
}

//新加
int count_page(int pages[]) { 
    int count = 0;
    for (int i = 0; i < 512; i++) {
        if (pages[i] == 0) {
            count++;
        }
    }
    return count;
}

//新加


int first_fit(struct Process* process, int* memory);
int page_fit(struct Process* process, int* pages, struct Process** ready_queue, int time);



//新加
int page_fit(struct  Process* process, int* pages, struct  Process** ready_queue, int time) { // Add 'struct' before the type names 'process_t' and 'list_t', and fix the parameter types
    const int PAGE_SIZE = 4;
    int num_frame = ceil((double)process->memory_requirement / PAGE_SIZE);
    int available_pages = count_page(pages);

    void evict_page(int num_frame, struct Process** ready_queue, int* pages, int time); // Add the missing function declaration

    if (available_pages < num_frame) {
        evict_page(num_frame, ready_queue, pages, time); // Fix the parameter names
    }
    int i, count = 0;
    for (i = 0; i < 512; i++) {
        if (pages[i] == 0) {
            process->frame[count] = i;
            count++;
            pages[i] = 1;
            if(count == num_frame){
                return 1;
            }
        }
    }
    return count == num_frame ? 1 : 0;
}

//新加
void evict_page(int num_frame, struct Process** ready_queue, int* pages, int time);

int has_printed_eviction_info = 0;
void reset_eviction_print_flag() {
    has_printed_eviction_info = 0; // Reset the print flag at the start of a new cycle or as needed
}
void evict_page(int num_frame, struct Process** ready_queue, int* pages, int time) { 
    while(count_page(pages)< num_frame){
       struct Process* head = *ready_queue; 
        if (head->frame == NULL) { 
            continue;
        }

        if (!has_printed_eviction_info && head->num_page > 0) {
            printf("/%d/, EVICTED, evicted-frame = [", time);
            for (int i = 0; i < head->num_page; i++) {
                int index = head->frame[i];
                printf("%d", index);
                if (i < head->num_page - 1) {
                    printf(", ");
                }
                pages[index] = 0; // Evict the page
            }
            printf("]\n");
            has_printed_eviction_info = 1; // Set the flag after printing
        } else {
            for (int i = 0; i < head->num_page; i++) {
                pages[head->frame[i]] = 0; // Mark the page as free
            }
        }
        
        head->num_page = 0; 
        aHead_to_bTail(ready_queue, &head); // Move the process to the tail of the ready queue
        *ready_queue = head; // Update the head of the queue
    }




struct MemoryBlock *initialize_memory(int memory_capacity){
    struct MemoryBlock *memory = malloc(sizeof(struct MemoryBlock));
    if (memory == NULL){
        exit(EXIT_FAILURE);
    }
    memory->start_address = 0;
    memory->length = memory_capacity;
    memory->next = NULL;
    memory->is_allocated = 0;
    return memory;
}

int allocate_memory(struct MemoryBlock **memory, struct Process **running_process){
    struct MemoryBlock *current = *memory;
    struct Process *current_running_process = *running_process;
    if(current_running_process != NULL){
        while (current != NULL){
            if (!current->is_allocated && current->length >= current_running_process->memory_requirement){
                if(current->length == current_running_process->memory_requirement){
                    current->is_allocated = 1;
                }else{
                    // order: prev current new_block next
                    struct MemoryBlock *new_block = malloc(sizeof(struct MemoryBlock));
                    if (new_block == NULL){
                        exit(EXIT_FAILURE);
                    }new_block->start_address = current->start_address + current_running_process->memory_requirement;
                    new_block->length = current->length - current_running_process->memory_requirement;
                    new_block->is_allocated = 0;
                    new_block->next = current->next;

                    current->length = current_running_process->memory_requirement;
                    current->is_allocated = 1;
                    current->next = new_block;
                }strcpy(current->process_name, current_running_process->name);
                return 1;
            }current = current->next;
        }return 0;
    }return 0;
}

void deallocate_memory(struct Process **running_process, struct MemoryBlock **memory,char* memory_strategy){
    struct Process *current_running_process = *running_process;
    struct MemoryBlock *current = *memory;
    struct MemoryBlock *prev = NULL;
    struct MemoryBlock *next_block = NULL;

    
    // Check if using paged strategy and handle accordingly
    if (strcmp(memory_strategy, "paged") == 0) {
        for (int i = 0; i < current_running_process->num_page; i++) {
            pages[current_running_process->frame[i]] = 0; 
        }
        current_running_process->num_page = 0; 
        return; 
    }

    while(current != NULL && strcmp(current->process_name,current_running_process->name) != 0){
        prev = current;
        current = current->next;
    }

    if (current == NULL){
        return;
    }
    current->is_allocated = 0;

    // merge prev block
    if (prev != NULL && !prev->is_allocated) {
        prev->length += current->length;
        prev->next = current->next;
        free(current);
        current = prev;
    }

    // merge next block
    if(current->next != NULL && !current->next->is_allocated){
        next_block = current->next;
        strcpy(current->process_name, "");
        current->length += next_block->length;
        current->next = next_block->next;
        free(next_block);
    }
}

int in_memory(struct MemoryBlock **memory, struct Process **running_processes){
    struct MemoryBlock *memory_head = *memory;
    struct Process *current = *running_processes;
    while(memory_head != NULL){
        if (memory_head->is_allocated == 1 && strcmp(memory_head->process_name, current->name) == 0){
            return 1;
        }else{
            memory_head = memory_head->next;
        }
    }return 0;

}
// 新加
int allocate_memory(struct MemoryBlock *memory, struct Process *running_process) {
    struct MemoryBlock *current = memory;
    while (current != NULL) {
        if (!current->is_allocated && current->length >= running_process->memory_requirement) {
            current->is_allocated = 1;
            strcpy(current->process_name, running_process->name);
            return 1;  // 成功分配内存
        }
        current = current->next;
    }
    return 0;  // 未能分配内存
}
// 新加
void process_manager(struct MemoryBlock **memory, struct Process **running_processes, struct Process **waiting_processes, int quantum, char *last_process_name, char *memory_strategy, int *pages) {
    int check = 0;
    while (check != 1 && *running_processes != NULL) {
        struct Process *current_running_process = *running_processes;
        struct MemoryBlock *memory_head = *memory;

        if (strcmp(memory_strategy, "infinite") == 0) {
            check = 1;  // 如果内存策略是无限，假设总是有足够内存
        } else if (strcmp(memory_strategy, "first-fit") == 0) {
            // 尝试按 first-fit 策略分配内存
            int in_memory(struct MemoryBlock *memory, struct Process *running_processes);
        } else if (strcmp(memory_strategy, "paged") == 0) {
            // 尝试按 paged 策略分配内存
            if (page_fit(current_running_process, pages, waiting_processes, current_running_process->arrival_time)) {
                check = 1;
            }
        } else {


            if (allocate_memory(memory_head, current_running_process)) {
                check = 1;
            }else{
            strcpy(last_process_name, current_running_process->name);
            aHead_to_bTail(running_processes, waiting_processes);
            *running_processes = current_running_process;  
        }
    }
}


int find_memory_address(struct MemoryBlock **memory, char* name){
    struct MemoryBlock *memory_head = *memory;
    while(memory_head != NULL){
        if (memory_head->is_allocated == 1 && strcmp(memory_head->process_name, name) == 0){
            return memory_head->start_address;
        }else{
            memory_head = memory_head->next;
        }
    }return MEMORY_CAPACITY + 1;
}

/*
//新加
print：arrival time，EVICTED，evicted-frame =【】
1.当内存不足时释放
2.当finish时释放
evicted-frame =【】这个array去循环array里面的每一个数字，一个一个print
设一个flag variable，当print了一次之后，flag = 1，下次再evict的时候，flag = 0，不再print

*/
void round_robin(struct Process **processes, struct PerformStat **statistics, struct MemoryBlock **memory, int num_process, char *memory_strategy, int quantum, int *simulation_time){
    struct Process *waiting_processes = *processes;
    struct Process *running_processes = NULL;
    struct MemoryBlock *memory_head = *memory;
    char last_process_name[FILENAME_LENGTH] = {0};
    int time_counter = 0;
    int num_process_done = 0;

    while (num_process_done < num_process){
        sort_by_arrival_time(&waiting_processes);

        // Getting all the processes that arrived
        struct Process* current_waiting_process = waiting_processes;
        while (current_waiting_process != NULL){   
            if (current_waiting_process->arrival_time <= time_counter){
                current_waiting_process->state = READY; 
                aHead_to_bTail(&current_waiting_process, &running_processes);
                waiting_processes = current_waiting_process->next;
            }
            current_waiting_process = current_waiting_process->next;
        }
        process_manager(&memory_head, &running_processes, &waiting_processes, quantum, last_process_name, memory_strategy, pages); // All required arguments included

        if (running_processes != NULL){
            running_processes->state = RUNNING;
            if(strcmp(last_process_name, running_processes->name) != 0){
                printf("%d,%s,process-name=%s,remaining-time=%d,mem-usage=%.0f%%,allocated-at=%d\n", time_counter, stateToString(running_processes->state), running_processes->name, running_processes->service_time, mem_usage(memory), find_memory_address(memory, running_processes->name));
            } else {
                printf("%d,%s,process-name=%s,remaining-time=%d\n", time_counter, stateToString(running_processes->state), running_processes->name, running_processes->service_time);
            }

            strcpy(last_process_name, running_processes->name);
            if ((running_processes->service_time - quantum) > 0){
                running_processes->service_time -= quantum; 
                running_processes->arrival_time = quantum + time_counter;
                aHead_to_bTail(&running_processes, &waiting_processes);
                time_counter += quantum;
            } else {
                // Finish at the end or before quantum
                running_processes->state = FINISHED;
                time_counter += running_processes->service_time;
                num_process_done++;
                printf("%d,%s,process-name=%s,proc-remaining=%d\n", time_counter, stateToString(running_processes->state), running_processes->name, ready_process_num(statistics, time_counter) - num_process_done);              
                update_stat_data(statistics, running_processes->name, time_counter);
                *simulation_time = time_counter;
                deallocate_memory(&running_processes, &memory_head, memory_strategy);
                remove_head(&running_processes);
            }
        } else {
            time_counter += quantum;
        }
    }
}


/*
//new, addr is for frame, beacuuase frae means the proc corresponds to the frame
get_arr_list(input_queue,ready_queue,time);
while(! is_empty (input_queue)|| ! is empty(ready_queue)){
    struct Process *head = NULL;
    while(1){
        head =get_head(ready_queue);
        if(head == NULL){
            break;
        }
        if(head -> addr!= -1 && strcmp(strategy,"first-fit")==0){
            break;
        }
        int sucess = -1;
        if(strcmp(strategy,"infinite")== 0){
            break;
        }else if (strategy, "first-fit")==0 {
            sucess = first_fit(head, memory);
    }else if((strategy,"paged")==0){
        sucess = page_fit(head, pages, ready_queue, time);
    }
    if (success ==1){
        break;
    }
    insert_at_foot(ready_queue, head);
    }
}
*/










void print_stat(struct PerformStat **head, int *simulation_time){
    float tot_tuenaround = 0;
    float tot_overhead = 0;
    float max_overhead = 0;
    int len = 0;
    if (*head != NULL){
        struct PerformStat *current = *head;
        while(current != NULL){
            len++;
            tot_tuenaround += current->turnaround_time;
            tot_overhead += current->overhead_time;
            if(current->overhead_time > max_overhead){
                max_overhead = current->overhead_time;
            }current = current->next;
        }
    }
    printf("Turnaround time %.0f\nTime overhead %.2f %.2f\nMakespan %d\n", ceil(tot_tuenaround/len), max_overhead/DIVISOR, tot_overhead/len/DIVISOR, *simulation_time);

}

void free_processes(struct Process *processes){
    free(processes);
}

void free_stat(struct PerformStat *stat){
    free(stat);
}

int main(int argc, char* argv[]){
    // read input line
    char filename[FILENAME_LENGTH];
    char memory_strategy[MEMORY_STRATEGY_LENGTH];
    int quantum;
    int simulation_time = 0;
    
    get_input_line(argc, argv, filename, memory_strategy, &quantum);

    struct Process *processes = NULL;
    struct PerformStat *statistics = NULL;
    struct MemoryBlock *memory = initialize_memory(MEMORY_CAPACITY);
    int num_processes = read_input_file(filename, &processes, &statistics);
    

    void round_robin(struct Process **processes, struct PerformStat **statistics, struct MemoryBlock **memory, int num_processes, char *memory_strategy, int quantum, int *simulation_time);

    round_robin(&processes, &statistics, &memory, num_processes, memory_strategy, quantum, &simulation_time);
    print_stat(&statistics, &simulation_time);
    
    free_processes(processes);
    free_stat(statistics);
            return 0;
        }
