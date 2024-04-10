
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#define FILENAME_LENGTH 100
#define PAGE_SIZE 4 // KB
#define TOTAL_MEMORY 2048 // KB
#define TOTAL_FRAMES (TOTAL_MEMORY / PAGE_SIZE)
#define PROCESS_NAME_LENGTH 100
#define DIVISOR 100
// 新的EVICTED状态
typedef enum {
    READY,
    RUNNING,
    FINISHED,
    EVICTED,
} ProcessState;

// 进程结构
struct Process {
    char name[PROCESS_NAME_LENGTH];
    int arrival_time;
    int service_time;
    int memory_requirement; // KB
    ProcessState state;
    int page_count;
    int* frames; // 存储分配给进程的帧编号
    struct Process* next;
    int last_executed_time;
} Process;

struct PerformStat{
    char name[FILENAME_LENGTH];
    int arrival_time;
    int service_time;
    int completion_time;
    int turnaround_time;
    float overhead_time;
    struct PerformStat *next;
};

struct MemoryFrame {
    int occupied; // 标记帧是否已被分配
    char process_name[FILENAME_LENGTH]; // 分配给该帧的进程名称
    int frame_number; // 帧编号
    struct MemoryFrame *next; // 指向下一个帧的指针
};

struct MemoryFrame memory[TOTAL_FRAMES]; // 声明内存帧数组


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

void update_stat_data(struct PerformStat **head, char* name, int completion_time) {
    if (*head != NULL) {
        struct PerformStat *current = *head;
        while (current != NULL) {
            if (strcmp(name, current->name) == 0) {
                current->completion_time = completion_time;
                current->turnaround_time = current->completion_time - current->arrival_time;
                // 确保服务时间不为零
                if (current->service_time > 0) {
                    current->overhead_time = (float)current->turnaround_time / current->service_time;
                } else {
                    // 处理服务时间为零的情况，根据实际需求调整
                    current->overhead_time = 0;
                }
                break; // 找到后就可以退出循环
            }
            current = current->next;
        }
    }
}


int read_input_file(char *filename, Process **processes) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    int num_process = 0;
    int arrival_time, service_time, memory_requirement;
    char name[FILENAME_LENGTH];

    while (fscanf(file, "%d %s %d %d", &arrival_time, name, &service_time, &memory_requirement) == 4) {
        Process *new_process = (Process *)malloc(sizeof(Process));
        if (new_process == NULL) {
            fprintf(stderr, "Error: Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
        new_process->arrival_time = arrival_time;
        strcpy(new_process->name, name);
        new_process->service_time = service_time;
        new_process->memory_requirement = memory_requirement;
        new_process->page_count = (memory_requirement + PAGE_SIZE - 1) / PAGE_SIZE; // 计算所需页数
        new_process->frames = (int*)malloc(new_process->page_count * sizeof(int)); // 分配存储帧编号的数组
        new_process->next = NULL;
        // 将新进程添加到进程列表
       
        add_process_to_list(processes, new_process);

        num_process++;
    }

    fclose(file);
    return num_process;
}


float calculateMemoryUsage() {
    int occupiedFrames = 0;
    for (int i = 0; i < TOTAL_FRAMES; ++i) {
        if (memory[i].occupied) {
            ++occupiedFrames;
        }
    }
    return (occupiedFrames * 100.0) / TOTAL_FRAMES;
}

void print_processes(struct Process *head) {
    struct Process *current = head;
    printf("Current processes:\n");
    while (current != NULL) {
        printf("Arrival Time: %d, Name: %s, Service Time: %d, Memory Requirement: %d KB\n",
               current->arrival_time, current->name, current->service_time, current->memory_requirement);
          current = current->next;
    }
}

void print_memory_frames(struct MemoryFrame memory[], int total_frames) {
    printf("Current Memory Frame Usage:\n");
    for (int i = 0; i < total_frames; ++i) {
        if (memory[i].occupied) {
            printf("Frame %d: Occupied by process %s\n", i, memory[i].process_name);
        } else {
            printf("Frame %d: Free\n", i);
        }
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
    switch (state) {
        case READY:
            return "READY";
        case RUNNING:
            return "RUNNING";
        case FINISHED:
            return "FINISHED";
        case EVICTED:
            return "EVICTED";
        default:
            return "UNKNOWN";
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



void allocateOrSwapPages(struct Process* process) {
    if (!arePagesAllocated(process)) {
        if (!allocatePages(process)) {
            evictPages();
            allocatePages(process);
        }
    }
}








// 功能函数声明
void initialize_memory();
int allocate_pages(Process* process);
void evict_pages();
void schedule_process(Process* process);
void update_process_state(Process* process, ProcessState new_state);
void print_memory_usage();
float calculateMemoryUsage();

void round_robin_scheduling() {
    struct Process *waiting_processes = *processes;
    struct Process *running_processes = NULL;
    struct Process* process_list_head = NULL;
    struct MemoryBlock *memory_head = *memory;
    char last_process_name[FILENAME_LENGTH];
    int time_counter = 0;
    int num_process_done = 0;
    while (num_process_done < num_process){
 
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
        process_manager(&memory_head, &running_processes, &waiting_processes, quantum, last_process_name);

        // do one process and put it back to waiting list
        if (running_processes != NULL){
            running_processes->state = RUNNING;
            if(strcmp(last_process_name, running_processes->name) != 0){
                if(strcmp(memory_strategy, "infinite") != 0){
                    printf("%d,%s,process-name=%s,remaining-time=%d,mem-usage=%.0f%%,allocated-at=%d\n", time_counter, stateToString(running_processes->state), running_processes->name, running_processes->service_time, mem_usage(memory), find_memory_address(memory, running_processes->name));
                }else{
                    printf("%d,%s,process-name=%s,remaining-time=%d\n", time_counter, stateToString(running_processes->state), running_processes->name, running_processes->service_time);
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
                printf("%d,%s,process-name=%s,proc-remaining=%d\n", time_counter, stateToString(running_processes->state), running_processes->name, ready_process_num(statistics, time_counter) - num_process_done);              
                update_stat_data(statistics, running_processes->name, time_counter);
                *simulation_time = time_counter;
                deallocate_memory(&running_processes, &memory_head);
                remove_head(&running_processes);
            }
        }else{
            time_counter += quantum;
        }
        
    }

}

void get_input_line(int argc, char* argv[], char *filename, char* memory_strategy, int* quantum) {
    *quantum = 0;
    for (int i = 1; i < argc; i+=2) {
        if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            strcpy(filename, argv[i + 1]);
        } else if (strcmp(argv[i], "-m") == 0 && i + 1 < argc) {
            strcpy(memory_strategy, argv[i + 1]);
        } else if (strcmp(argv[i], "-q") == 0 && i + 1 < argc) {
            *quantum = atoi(argv[i + 1]);
        }
    }
}







void initialize_memory() {
    for (int i = 0; i < TOTAL_FRAMES; i++) {
        memory[i].occupied = 0; // 标记帧为未分配
        strcpy(memory[i].process_name, ""); // 初始化进程名称为空
        memory[i].frame_number = i; // 设置帧编号
    }
}

void enqueue(Process** queue, Process* process) {
    if (*queue == NULL) {
        *queue = process;
    } else {
        Process* temp = *queue;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = process;
    }
    process->next = NULL;
}

Process* dequeue(Process** queue) {
    if (*queue == NULL) {
        return NULL;
    } else {
        Process* process = *queue;
        *queue = (*queue)->next;
        process->next = NULL;
        return process;
    }
}

int are_pages_allocated(Process* process) {
    for (int i = 0; i < process->page_count; i++) {
        if (process->frames[i] == -1) { // 假设-1表示该页未被分配
            return 0;
        }
    }
    return 1;
}

int allocate_pages(struct Process* process) {
    int required_frames = process->page_count;
    int allocatedFrames = 0;

    if (count_free_frames() < required_frames) {
        evict_pages();
    }

    int pages_allocated = 0;
    for (int i = 0; i < TOTAL_FRAMES && pages_allocated < required_frames; i++) {
        if (!memory[i].occupied) {
            memory[i].occupied = 1;
            strcpy(memory[i].process_name, process->name);
            process->frames[pages_allocated++] = i;
        }
    }
    return allocatedFrames == requiredFrames;
}


void free_pages(Process* process) {
    for (int i = 0; i < process->page_count; i++) {
        if (process->frames[i] != -1) { // 如果该页已被分配
            memory[process->frames[i]].occupied = 0;
            strcpy(memory[process->frames[i]].process_name, "");
            process->frames[i] = -1; // 标记为未分配
        }
    }
}

void simulate_process_execution(Process* process, int quantum) {
    process->service_time -= quantum;
    if (process->service_time < 0) {
        process->service_time = 0;
    }
}

void evict_pages(int required_free_frames) {
    while (count_free_frames() < required_free_frames) {
        struct Process* lru_process = find_least_recently_used_process();
        if (lru_process != NULL) {
            for (int i = 0; i < lru_process->page_count; i++) {
                int frame = lru_process->frames[i];
                if (frame != -1) { // Ensure the frame was allocated
                    memory[frame].occupied = 0;
                    strcpy(memory[frame].process_name, "");
                    lru_process->frames[i] = -1;
                }
            }
            lru_process->state = EVICTED;
            printf("%d,EVICTED,evicted-frames=[", currentTime); // currentTime needs to be managed globally or passed as a parameter
            for (int i = 0; i < lru_process->page_count; ++i) {
                if (i > 0) printf(", ");
                printf("%d", lru_process->frames[i]);
            }
            printf("]\n");
        } else {
            // If no process is found for eviction, this is a critical error.
            fprintf(stderr, "Error: No process found for eviction.\n");
            exit(EXIT_FAILURE);
        }
    }
}

void schedule_process(struct Process* process, int current_time) {
    switch (process->state) {
        case READY:
            // 尝试为进程分配内存页
            if (!allocate_pages_to_process(process)) {
                // 如果分配失败，尝试驱逐其他进程的页
                evict_pages();
                // 在尝试驱逐页后，再次尝试为当前进程分配内存页
                if (!allocate_pages_to_process(process)) {
                    printf("Failed to allocate memory pages for process %s\n", process->name);
                    return;
                }
            }
            // 更新进程状态为RUNNING，设置最后执行时间
            process->state = RUNNING;
            process->last_executed_time = current_time;
            printf("%d,RUNNING,process-name=%s,remaining-time=%d,mem-usage=%d%%,mem-frames=[",
                   current_time, process->name, process->service_time, calculateMemoryUsage());
            for (int i = 0; i < process->page_count; ++i) {
                printf("%d", process->frames[i]);
                if (i < process->page_count - 1) printf(", ");
            }
            printf("]\n");
            break;

        case RUNNING:
            // 进程执行一定时间后的处理
            process->service_time -= quantum; // 假设quantum是全局变量或已传入
            if (process->service_time <= 0) {
                // 如果进程已完成执行
                process->state = FINISHED;
                process->service_time = 0; // 确保服务时间不会是负数
                printf("%d,FINISHED,process-name=%s,proc-remaining=%d\n", 
                        current_time, process->name, /* 进程剩余数量 */);
                release_process_frames(process);
            } else {
                // 进程还未完成，需要继续执行
                process->last_executed_time = current_time;
                // 可能需要根据你的调度策略重新安排进程
            }
            break;

        case FINISHED:
            // 已完成的进程，此处可能无需操作
            // 确保释放占用的内存页
            release_process_frames(process);
            break;

        case EVICTED:
            // 被驱逐的进程尝试重新获得内存页
            if (!allocate_pages_to_process(process)) {
                printf("Failed to reallocate pages for evicted process %s\n", process->name);
            } else {
                process->state = READY; // 分配成功后，进程可能返回就绪队列
            }
            break;

        default:
            printf("Unknown state for process %s\n", process->name);
            break;
    }
}



struct Process* find_least_recently_used_process() {
    struct Process* result = NULL;
    int oldest_time = INT_MAX; // 假设INT_MAX为最大整数，代表时间的最大可能值

    // 遍历进程列表，找到最少最近使用的进程
    struct Process* current = process_list_head; // 假设process_list_head是进程列表的头指针
    while (current != NULL) {
        // 选择不在运行状态且最早执行的进程
        if (current->state != RUNNING && current->last_executed_time < oldest_time) {
            oldest_time = current->last_executed_time;
            result = current;
        }
        current = current->next;
    }

    return result;
}

void update_process_state(Process* process, ProcessState new_state) {
    process->state = new_state;
    switch (new_state) {
        case RUNNING:
            printf("%d,RUNNING,process-name=%s,remaining-time=%d,mem-usage=%d%%,mem-frames=[",
                   currentTime, process->name, process->service_time, calculateMemoryUsage());
            for (int i = 0; i < process->page_count; i++) {
                printf("%d", process->frames[i]);
                if (i < process->page_count - 1) printf(",");
            }
            printf("]\n");
            break;
        case FINISHED:
            process->endTime = currentTime;
            totalTurnaroundTime += process->endTime - process->startTime;
            printf("%d,FINISHED,process-name=%s,proc-remaining=%d\n", currentTime, process->name, --totalProcesses);
            if (totalProcesses == 0) {
                printSummary();
            }
            break;
}


void print_memory_usage() {
    int allocated_frames = 0;
    for (int i = 0; i < TOTAL_FRAMES; i++) {
        if (memory[i].occupied) {
            allocated_frames++;
            printf("Frame %d is allocated to process %s\n", i, memory[i].process_name);
        }
    }
    printf("Total allocated frames: %d\n", allocated_frames);
    printf("Total free frames: %d\n", TOTAL_FRAMES - allocated_frames);
    printf("Memory usage: %.2f%%\n", 100.0 * allocated_frames / TOTAL_FRAMES);
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
int main(int argc, char* argv[]) {
   
    char filename[FILENAME_LENGTH];
    char memory_strategy[MEMORY_STRATEGY_LENGTH];
    int quantum = 0;

    
    get_input_line(argc, argv, filename, memory_strategy, &quantum);
    initialize_memory();
    struct Process* processes = NULL;
    int num_processes = read_input_file(filename, &processes);
    round_robin_scheduling(&processes, quantum);

  

    return 0;
}