#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define FILENAME_LENGTH 100
#define MEMORY_STRATEGY_LENGTH 100
#define DIVISOR 100
#define MEMORY_CAPACITY 2048
#define MAX_PAGES 512
#define PAGE_SIZE 4
#define PAGE_LIMIT 4

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
};

struct Page{
    int page_num;
    int is_allocated;
    char name[FILENAME_LENGTH];
};

struct Page_Location {
    int data;
    struct Page_Location* next;
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

struct MemoryBlock{
    int is_allocated;// a hole or a process
    char process_name[FILENAME_LENGTH];
    int start_address;
    int length;
    struct MemoryBlock *next;
};

struct Page_Location* create_node(int data) {
    struct Page_Location* newNode = (struct Page_Location*)malloc(sizeof(struct Page_Location));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}

void test_pages(struct Page pages[]) {
    printf("Page List:\n");
    for (int i = 0; i < MAX_PAGES; i++) {
        printf("  Page Number: %d, ", pages[i].page_num);
        printf("  Is Allocated: %s, ", pages[i].is_allocated ? "Yes" : "No");
        printf("  Name: %s\n", pages[i].name);
        printf("\n");
    }
}

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

int page_count(struct Page pages[]){
    int counter = 0;
    for (int i = 0; i < MAX_PAGES; i++){
        if(pages[i].is_allocated == 0){
            counter++;
        }
    }return counter;
}

float ff_mem_usage(struct MemoryBlock **memory){
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

float paged_mem_usage(struct Page pages[]){
    float using_mem = 0;
    for(int i = 0; i < MAX_PAGES; i++){
        if (pages[i].is_allocated == 1){
            using_mem++;
        }
    }
    return ceil(using_mem*DIVISOR / MAX_PAGES);
    
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
/*
void aHead_to_aTail(struct Process **processes){
    struct Process* temp = *processes;
    if (temp->next != NULL && temp != NULL){
        struct Process *old_head = temp;
        *processes = temp -> next;
        struct Process *current = *processes;
        while(current -> next != NULL){
            current = current -> next;
        }current -> next = old_head;
        old_head->next = NULL;
    }

*/
char* find_matching_pages_as_string(struct Process** process, struct Page pages[]) {
    // Allocate memory for the string to store page numbers
    char* matched_pages_string = (char*)malloc(sizeof(char) * MAX_PAGES * 10 + 3); // Assuming each page number takes at most 10 characters
    if (matched_pages_string == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    strcpy(matched_pages_string, "["); // Start with opening square bracket
    int offset = 1;
    // Iterate through each page
    for (int i = 0; i < MAX_PAGES; i++) {
        // Check if the page is allocated and its name matches the process name
        if (pages[i].is_allocated && strcmp(pages[i].name, (*process)->name) == 0) {
            // Convert the page number to string and concatenate it to the matched_pages_string
            offset += sprintf(matched_pages_string + offset, "%d,", pages[i].page_num);
        }
    }

    // Replace the last comma and space with closing square bracket
    if (offset > 1) {
        matched_pages_string[offset - 1] = ']'; // Overwrite the comma with closing square bracket
        matched_pages_string[offset ] = '\0'; // Null-terminate the string
    } else {
        strcpy(matched_pages_string, "[]"); // No matches, return empty brackets
    }

    return matched_pages_string;
}

char* virtual_find_matching_pages_as_string(struct Process** process, struct Page pages[], int page_num_need) {
    // Allocate memory for the string to store page numbers
    char* matched_pages_string = (char*)malloc(sizeof(char) * MAX_PAGES * 10 + 3); // Assuming each page number takes at most 10 characters
    if (matched_pages_string == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    strcpy(matched_pages_string, "["); // Start with opening square bracket
    int offset = 1;
    int limit = page_count(pages);
    int tool = 0;
    if(page_num_need < PAGE_LIMIT){
        tool = page_num_need;
    }else{
        tool = PAGE_LIMIT;
    }

    for (int i = 0; i < MAX_PAGES && limit < tool; i++) {
        // Check if the page is allocated and its name matches the process name
        if (pages[i].is_allocated && strcmp(pages[i].name, (*process)->name) == 0) {
            // Convert the page number to string and concatenate it to the matched_pages_string
            offset += sprintf(matched_pages_string + offset, "%d,", pages[i].page_num);
            limit++;
        }
    }

    
    // Replace the last comma and space with closing square bracket
        if (offset > 1) {
         matched_pages_string[offset - 1] = ']'; // Overwrite the comma with closing square bracket
         matched_pages_string[offset ] = '\0'; // Null-terminate the string
        } else {
            strcpy(matched_pages_string, "[]"); // No matches, return empty brackets
        }

    

    return matched_pages_string;
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



void a_to_bTail(struct Process **a, struct Process **b) {
    if (a == NULL || *a == NULL) {
        // Nothing to append if list a is empty
        return;
    }

    // Create a new node for list b containing a copy of the data from the head of list a
    struct Process* newNode = (struct Process*)malloc(sizeof(struct Process));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    memcpy(newNode, *a, sizeof(struct Process));
    newNode->next = NULL;

    if (*b == NULL) {
        // If list b is empty, the new node becomes the head of b
        *b = newNode;
    } else {
        // Move tailB to the end of list b
        struct Process* tailB = *b;
        while (tailB->next != NULL) {
            tailB = tailB->next;
        }
        // Append the new node to the tail of b
        tailB->next = newNode;
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
            current->next = sorted; // Insert the current node at the beginning
            sorted = current; // Update the head of the sorted list
        } else {
            // Else, find the right position to insert the current node in the sorted list
            struct Process* traverse = sorted;
            while (traverse->next != NULL &&
                   (traverse->next->arrival_time < current->arrival_time ||
                    (traverse->next->arrival_time == current->arrival_time && traverse->next->memory_requirement <= current->memory_requirement))) {
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

int ff_allocate_memory(struct MemoryBlock **memory, struct Process **running_process){
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

int page_in_page(struct Page pages[], char *name){
    for (int i = 0; i < MAX_PAGES; i++){
        if(strcmp(pages[i].name, name) == 0){
            return 1;
        }
    }return 0;
}

int virtual_in_page(struct Page pages[], char *name, int num_page_need){
    int counter = 0;
    if (num_page_need < PAGE_LIMIT){
        for (int i = 0; i < MAX_PAGES; i++){
            if(strcmp(pages[i].name, name) == 0){
                counter++;
            }
        }if(counter == num_page_need){
            return 1;
        }
    }else{
        for (int i = 0; i < MAX_PAGES; i++){
            if(strcmp(pages[i].name, name) == 0){
                counter++;
            }if(counter == PAGE_LIMIT){
                return 1;
            }
        }
    }
    return 0;
}


void evict_page(struct Page pages[], int page_num_need, int time_counter, struct Process** process_history){
    int has_printed_eviction_info = 0;
    while(page_count(pages) < page_num_need && *process_history != NULL){
        for(int i = 0; i < MAX_PAGES; i++){
            if(strcmp(pages[i].name, (*process_history)->name) == 0){
                if (has_printed_eviction_info == 0){
                    printf("%d,EVICTED,evicted-frames=%s\n", time_counter, find_matching_pages_as_string(process_history, pages));
                    has_printed_eviction_info = 1;
                }strcpy(pages[i].name, "");
                pages[i].is_allocated = 0;
            }
        }*process_history = (*process_history)->next;
    }
}

void virtual_evict_page(struct Page pages[], int page_num_need, int time_counter, struct Process** process_history){
    int has_printed_eviction_info = 0;
    int tool = 0;
    struct Process* head = *process_history;
    if(page_num_need < PAGE_LIMIT){
        tool = page_num_need;
    }else{
        tool = PAGE_LIMIT;
    }
    while(page_count(pages) < tool && head != NULL){
        for(int i = 0; i < MAX_PAGES && page_count(pages) < tool; i++){
            if(strcmp(pages[i].name, head->name) == 0){
                if (has_printed_eviction_info == 0){
                    printf("%d,EVICTED,evicted-frames=%s\n", time_counter, virtual_find_matching_pages_as_string(&head, pages, page_num_need));
                    has_printed_eviction_info = 1;
                }strcpy(pages[i].name, "");
                pages[i].is_allocated = 0;
            }
        }head = head->next;
    }
}

void evict_finished_page(struct Page pages[], int page_num_need, int time_counter, struct Process** current_process){
    int has_printed_eviction_info = 0;
    for(int i = 0; i < MAX_PAGES; i++){
        if(strcmp(pages[i].name, (*current_process)->name) == 0){
            if (has_printed_eviction_info == 0){
                printf("%d,EVICTED,evicted-frames=%s\n", time_counter, find_matching_pages_as_string(current_process, pages));
                has_printed_eviction_info = 1;
            }strcpy(pages[i].name, "");
            pages[i].is_allocated = 0;
        }
    }
}


void page_allocate_memory(struct Page pages[], struct Process **running_process, struct Process **process_history, int time_counter){
    struct Process *current_running_process = *running_process;
    if(current_running_process != NULL){
        if(page_in_page(pages, current_running_process->name) == 1){
            return;
        }else{
            int page_num_need = ceil((double)current_running_process->memory_requirement / PAGE_SIZE);
            int page_available = page_count(pages);
            if(page_available < page_num_need){
                evict_page(pages, page_num_need, time_counter, process_history);
            }
            int counter = 0;
            for (int i = 0; i < MAX_PAGES && counter < page_num_need; i++){
                if(pages[i].is_allocated == 0){
                    pages[i].is_allocated = 1;
                    strcpy(pages[i].name, current_running_process->name);
                    counter++;
                }
            }
        }
    }
}

void virtual_allocate_memory(struct Page pages[], struct Process **running_process, struct Process **process_history, int time_counter){
    struct Process *current_running_process = *running_process;
    int page_num_need = ceil((double)current_running_process->memory_requirement / PAGE_SIZE);
    if(current_running_process != NULL){
        if(virtual_in_page(pages, current_running_process->name, page_num_need) == 1){
            return;
        }else{
            if((page_num_need < PAGE_LIMIT && page_count(pages) < page_num_need) || (page_num_need >= PAGE_LIMIT && page_count(pages) < PAGE_LIMIT)){
                virtual_evict_page(pages, page_num_need, time_counter, process_history);
            }
            int counter = 0;
            // page needed less than 4
            if (page_num_need < PAGE_LIMIT){
                for (int i = 0; i < MAX_PAGES && counter < page_num_need; i++){
                    if(pages[i].is_allocated == 0){
                        pages[i].is_allocated = 1;
                        strcpy(pages[i].name, current_running_process->name);
                        counter++;
                    }
                }
            }else{
                // page needed more than 4
                // available page less than needed, only give four frames
                if(page_count(pages) < page_num_need){
                    counter = 0;
                    int tool_counter = page_count(pages);
                    for (int i = 0; i < MAX_PAGES && counter < tool_counter; i++){
                        if(pages[i].is_allocated == 0){
                            pages[i].is_allocated = 1;
                            strcpy(pages[i].name, current_running_process->name);
                            counter++;
                        }
                    }
                }else{
                    // available page more than needed, give as many as possible
                    for (int i = 0; i < MAX_PAGES && counter < page_num_need; i++){
                        if(pages[i].is_allocated == 0){
                            pages[i].is_allocated = 1;
                            strcpy(pages[i].name, current_running_process->name);
                            counter++;
                        }
                    }
                }
                
            }
            
        }
    }
}

void ff_deallocate_memory(struct Process **running_process, struct MemoryBlock **memory){
    struct Process *current_running_process = *running_process;
    struct MemoryBlock *current = *memory;
    struct MemoryBlock *prev = NULL;
    struct MemoryBlock *next_block = NULL;
    char current_process_name[MEMORY_STRATEGY_LENGTH];
    char current_running_process_name[MEMORY_STRATEGY_LENGTH];
    // Copy process names into character arrays
    strcpy(current_process_name, current->process_name);
    strcpy(current_running_process_name, current_running_process->name);
    while(current != NULL && strcmp(current_process_name,current_running_process_name) != 0){
        prev = current;
        current = current->next;
        strcpy(current_process_name, current->process_name);
        strcpy(current_running_process_name, current_running_process->name);
        
    }

    if (current == NULL){
        return;
    }
    current->is_allocated = 0;

    // merge prev block
    if(prev != NULL && !prev->is_allocated){
        prev->length += current->length;
        prev->next = current->next;
        free(current);
        current = prev;
    }

    // merge next block
    if(current->next != NULL && !current->next->is_allocated){
        next_block = current->next;
        strcpy(current->process_name, "NULL");
        current->length += next_block->length;
        current->next = next_block->next;
        free(next_block);
    }
}

int ff_in_memory(struct MemoryBlock **memory, struct Process **running_processes){
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

int page_in_memory(struct Page pages[], struct Process **running_processes){
    struct Process *current = *running_processes;
    for (int i = 0; i < MAX_PAGES; i++){
        if (pages->is_allocated  == 1 && strcmp(pages->name, current->name) == 0){
            return 1;
        }
    }return 0;

}


void ff_process_manager(struct MemoryBlock **memory, struct Process **running_processes, struct Process **waiting_processes, int quantum, char *last_process_name){
    int check = 0;
    while (check != 1 && *running_processes != NULL){
        struct Process *current_running_process = *running_processes;
        struct MemoryBlock *memory_head = *memory;
        if (ff_in_memory(&memory_head, &current_running_process) == 1){
            check = 1;
        }else{
            if (ff_allocate_memory(&memory_head, &current_running_process) == 1){
                check = 1;
            }else{
            //    current_running_process->arrival_time += quantum;
                strcpy(last_process_name, current_running_process->name);
                aHead_to_bTail(&current_running_process, waiting_processes);
                *running_processes = current_running_process;
            }
        }
    }
}

void paged_process_manager(struct Page pages[], struct Process **running_processes, struct Process **process_history, struct Process **waiting_processes, int quantum, char *last_process_name, int time_counter){
    int check = 0;
    while (check != 1 && *running_processes != NULL){
        struct Process *current_running_process = *running_processes;
        if (page_in_memory(pages, &current_running_process) == 1){
            check = 1;
        }else{
            page_allocate_memory(pages, &current_running_process, process_history, time_counter);
            check = 1;
        }
    }
}

void virtual_process_manager(struct Page pages[], struct Process **running_processes, struct Process **process_history, struct Process **waiting_processes, int quantum, char *last_process_name, int time_counter){
    int check = 0;
    while (check != 1 && *running_processes != NULL){
        struct Process *current_running_process = *running_processes;
        if (page_in_memory(pages, &current_running_process) == 1){
            check = 1;
        }else{
            virtual_allocate_memory(pages, &current_running_process, process_history, time_counter);
            check = 1;
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

int in_history(char* name, struct Process** process_history){
    struct Process *current = *process_history;
    while(current != NULL){
        if (strcmp(name, current->name) == 0){
            return 1;
        }else{
            current = current->next;
        }
    }return 0;
}

void append_process(char* process_name, struct Process** process_history, struct Process** running_process) {
    if (*process_history == NULL || process_name == NULL) {
        // Invalid input or empty list, no action needed
        return;
    }

    struct Process* current = *process_history;
    struct Process* prev = NULL;

    // Traverse the list to find the node with the specified name
    while (current != NULL) {
        if (strcmp(current->name, process_name) == 0) {
            break;
        }else{
            prev = current;
            current = current->next;
        }
        
    }

    if (current == NULL) {
        // Node with the specified name not found in the list, will not happen
        return;
    }

    // If the node to move is already at the end, no action needed
    if (current->next == NULL) {
        return;
    }

    // Detach the node to move from its current position
    if (prev != NULL) {
        prev->next = current->next;
    } else {
        *process_history = current->next;
    }

    // Find the last node in the list
    struct Process* head = *process_history;
    while (head->next != NULL) {
        head = head->next;
    }
    
    // Create a new node and copy all the information
    struct Process* new_node = (struct Process*)malloc(sizeof(struct Process));
    if (new_node == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    memcpy(new_node, current, sizeof(struct Process));
    new_node->next = NULL;
    // Append the new node to the end of the list
    head->next = new_node;


}

void remove_tail(struct Process **head) {
    if (*head == NULL || (*head)->next == NULL) {
        // List is empty or has only one node (tail is head)
        // No action needed or cannot remove the only node
        return;
    }

    struct Process *current = *head;
    struct Process *prev = NULL;

    // Traverse the list until the second-to-last node
    while (current->next != NULL) {
        prev = current;
        current = current->next;
    }

    // Update the second-to-last node's next pointer to NULL
    prev->next = NULL;

    // Free the memory occupied by the last node
    free(current);
}


void update_history_process(struct Process** running_process, struct Process** process_history){
    struct Process *current = *process_history;
    struct Process *running_head = *running_process;
    
    if (in_history(running_head->name, process_history) == 1){
        while(current != NULL){
            if (strcmp(running_head->name, current->name) == 0){
                char* name = running_head->name;
                append_process(name, process_history, running_process);
             //   remove_tail(running_process);
              //  print_processes(*running_process); 878
                return;
            }else{
                current = current->next;
            }
        }

    }else{
        a_to_bTail(&running_head, process_history);
        
    }
    

}

void round_robin(struct Process **processes, struct Process **process_history, struct PerformStat **statistics, struct MemoryBlock **memory, int num_process, char *memory_strategy, int quantum, int *simulation_time, struct Page pages[]){
    struct Process *waiting_processes = *processes;
    struct Process *running_processes = NULL;
    struct MemoryBlock *memory_head = *memory;
    char last_process_name[FILENAME_LENGTH] = "";
    int time_counter = 0;
    int num_process_done = 0;
    for (int i = 0; i < MAX_PAGES; i++){
        pages[i].page_num = i;
        pages[i].is_allocated = 0;
        strcpy(pages[i].name, "");
    }
    while (num_process_done < num_process){

   // for  (int i = 0; i < 5; i++){
        sort_by_arrival_time(&waiting_processes);
        // getting all the processes that arrived
        struct Process* current_waiting_process = waiting_processes;
        while (current_waiting_process != NULL){   
            if (current_waiting_process->arrival_time <= time_counter){
                current_waiting_process->state = READY; 
                aHead_to_bTail(&current_waiting_process, &running_processes);
                waiting_processes = current_waiting_process;
            }else{
                current_waiting_process = current_waiting_process -> next;
            }
        }
        if(strcmp(memory_strategy, "first-fit") == 0){
            ff_process_manager(&memory_head, &running_processes, &waiting_processes, quantum, last_process_name);
        }else if(strcmp(memory_strategy, "paged") == 0){
            paged_process_manager(pages, &running_processes, process_history, &waiting_processes, quantum, last_process_name, time_counter);
        }else if(strcmp(memory_strategy, "virtual") == 0){
            virtual_process_manager(pages, &running_processes, process_history, &waiting_processes, quantum, last_process_name, time_counter);
        }else{
        }
        // do one process and put it back to waiting list
        
        if (running_processes != NULL){
            running_processes->state = RUNNING;
            if(strcmp(last_process_name, running_processes->name) != 0){
                if(strcmp(memory_strategy, "infinite") == 0){
                    printf("%d,%s,process-name=%s,remaining-time=%d\n", time_counter, stateToString(running_processes->state), running_processes->name, running_processes->service_time);
                }else if(strcmp(memory_strategy, "first-fit") == 0){
                    printf("%d,%s,process-name=%s,remaining-time=%d,mem-usage=%.0f%%,allocated-at=%d\n", time_counter, stateToString(running_processes->state), running_processes->name, running_processes->service_time, ff_mem_usage(memory), find_memory_address(memory, running_processes->name));
                }else{
                    printf("%d,%s,process-name=%s,remaining-time=%d,mem-usage=%.0f%%,mem-frames=%s\n", time_counter, stateToString(running_processes->state), running_processes->name, running_processes->service_time, paged_mem_usage(pages), find_matching_pages_as_string(&running_processes, pages));
                    update_history_process(&running_processes, process_history);

                }
                
            }
            strcpy(last_process_name, running_processes->name);
            if ((running_processes->service_time - quantum) > 0){

                running_processes->service_time -= quantum;
                running_processes->arrival_time = quantum + time_counter;

                aHead_to_bTail(&running_processes, &waiting_processes);

                time_counter += quantum;
            }else{
                // finish at the end or before quantum
                running_processes->state = FINISHED;
                time_counter += quantum;
                num_process_done++;
                if (strcmp(memory_strategy, "paged") == 0 || strcmp(memory_strategy, "virtual") == 0){
                    evict_finished_page(pages, ceil((double)running_processes->memory_requirement / PAGE_SIZE), time_counter, &running_processes);
                }
                printf("%d,%s,process-name=%s,proc-remaining=%d\n", time_counter, stateToString(running_processes->state), running_processes->name, ready_process_num(statistics, time_counter) - num_process_done);              
                update_stat_data(statistics, running_processes->name, time_counter);
                *simulation_time = time_counter;
                if(strcmp(memory_strategy, "first-fit") == 0){
                    ff_deallocate_memory(&running_processes, &memory_head);
                }
            
                remove_head(&running_processes);
                
            }
        }else{
            time_counter += quantum;
        }
        
    }

}

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
    struct Process *process_history = NULL;
    struct PerformStat *statistics = NULL;
    struct MemoryBlock *memory = initialize_memory(MEMORY_CAPACITY);
    struct Page page_list[MAX_PAGES];
    int num_processes = read_input_file(filename, &processes, &statistics);
    round_robin(&processes, &process_history, &statistics, &memory, num_processes, memory_strategy, quantum, &simulation_time, page_list);
    print_stat(&statistics, &simulation_time);
    
    free_processes(processes);
    free_stat(statistics);
    free(memory);
    return 0;
}
//./allocate -f spec.txt -q 1 -m infinite
// ./allocate -f fill.txt -q 3 -m first-fit
//./allocate -f internal-frag.txt -q 1 -m paged
//./allocate -f no-evict.txt -q 3 -m virtual
