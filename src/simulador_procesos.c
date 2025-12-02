#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define MAX_PROCESSES 50
#define MAX_CPU 1
#define MAX_MEMORY_BLOCKS 4  // 4 GB en bloques de 1GB
#define MEMORY_BLOCK_SIZE 1024  // 1GB = 1024MB
#define MAX_MESSAGES 100
#define MAX_SEMAPHORES 10
#define BUFFER_SIZE 5
#define MAX_LOG_ENTRIES 1000

typedef enum {
    READY,
    RUNNING,
    WAITING,
    TERMINATED
} ProcessState;

typedef enum {
    NORMAL,
    ERROR_TERMINATION,
    DEADLOCK,
    USER_TERMINATION
} TerminationCause;

typedef enum {
    FCFS,
    ROUND_ROBIN
} SchedulingAlgorithm;

typedef struct {
    int cpu;  // 0 o 1
    int memory_blocks;  // bloques de 1GB
} Resources;

typedef struct {
    int pid;
    ProcessState state;
    int priority;
    Resources allocated;
    Resources needed;
    int burst_time;  // tiempo total de ejecucion
    int remaining_time;  // tiempo restante
    int arrival_time;  // tiempo de llegada
    int completion_time;  // tiempo de finalizacion
    int waiting_time;  // tiempo en espera
    int turnaround_time;  // tiempo de retorno
    int quantum_remaining;  // para Round Robin
    TerminationCause termination_cause;
} PCB;

typedef struct {
    int sender_pid;
    int receiver_pid;
    char content[256];
    bool delivered;
} Message;

typedef struct {
    int id;
    int value;
    int waiting_pids[MAX_PROCESSES];
    int waiting_count;
} Semaphore;

typedef struct {
    int items[BUFFER_SIZE];
    int count;
    int in;
    int out;
} SharedBuffer;

typedef struct {
    int time;
    int pid;
    char event[256];
} LogEntry;

typedef struct {
    PCB processes[MAX_PROCESSES];
    int process_count;
    int next_pid;
    
    Resources available;
    Resources total;
    
    Message messages[MAX_MESSAGES];
    int message_count;
    
    Semaphore semaphores[MAX_SEMAPHORES];
    int semaphore_count;
    
    SharedBuffer buffer;
    int mutex;  // semaforo mutex
    int empty;  // semaforo empty
    int full;   // semaforo full
    
    SchedulingAlgorithm algorithm;
    int quantum;
    int current_time;
    int running_pid;  // -1 si no hay proceso ejecutando
    
    LogEntry logs[MAX_LOG_ENTRIES];
    int log_count;
    
    int total_processes_completed;
    int total_waiting_time;
    int total_turnaround_time;
    int cpu_busy_time;
} System;

System sys;
int last_preempted_pid = -1;

void init_system();
void add_log(int pid, const char *event);
int create_process(int burst_time, int priority, int memory_blocks);
void suspend_process(int pid);
void resume_process(int pid);
void terminate_process(int pid, TerminationCause cause);
void list_processes();
void show_resources();
void show_logs();
void show_statistics();
bool request_resources(int pid, Resources req);
void release_resources(int pid);
bool check_deadlock_prevention(int pid, Resources req);
void execute_step();
void select_next_process();
int find_process_by_pid(int pid);
void send_message(int sender_pid, int receiver_pid, const char *content);
void receive_message(int receiver_pid);
int create_semaphore(int initial_value);
void wait_semaphore(int sem_id, int pid);
void signal_semaphore(int sem_id);
void demonstrate_producer_consumer();
void producer(int pid);
void consumer(int pid);
void show_menu();
void select_algorithm();


void init_system() {
    sys.process_count = 0;
    sys.next_pid = 1;
    sys.available.cpu = MAX_CPU;
    sys.available.memory_blocks = MAX_MEMORY_BLOCKS;
    sys.total.cpu = MAX_CPU;
    sys.total.memory_blocks = MAX_MEMORY_BLOCKS;
    sys.message_count = 0;
    sys.semaphore_count = 0;
    sys.current_time = 0;
    sys.running_pid = -1;
    sys.log_count = 0;
    sys.total_processes_completed = 0;
    sys.total_waiting_time = 0;
    sys.total_turnaround_time = 0;
    sys.cpu_busy_time = 0;
    sys.quantum = 2;
    
    sys.buffer.count = 0;
    sys.buffer.in = 0;
    sys.buffer.out = 0;
    
    last_preempted_pid = -1;
    
    add_log(-1, "Sistema inicializado");
}

void add_log(int pid, const char *event) {
    if (sys.log_count < MAX_LOG_ENTRIES) {
        sys.logs[sys.log_count].time = sys.current_time;
        sys.logs[sys.log_count].pid = pid;
        strncpy(sys.logs[sys.log_count].event, event, 255);
        sys.logs[sys.log_count].event[255] = '\0';
        sys.log_count++;
    }
}

int create_process(int burst_time, int priority, int memory_blocks) {
    if (sys.process_count >= MAX_PROCESSES) {
        printf("Error: Numero maximo de procesos alcanzado\n");
        return -1;
    }
    
    if (memory_blocks > MAX_MEMORY_BLOCKS) {
        printf("Error: Memoria solicitada excede el maximo (%d bloques de %dMB)\n", 
               MAX_MEMORY_BLOCKS, MEMORY_BLOCK_SIZE);
        return -1;
    }
    
    int idx = sys.process_count;
    PCB *p = &sys.processes[idx];
    
    p->pid = sys.next_pid++;
    p->state = READY;
    p->priority = priority;
    p->allocated.cpu = 0;
    p->allocated.memory_blocks = 0;
    p->needed.cpu = 1;
    p->needed.memory_blocks = memory_blocks;
    p->burst_time = burst_time;
    p->remaining_time = burst_time;
    p->arrival_time = sys.current_time;
    p->completion_time = 0;
    p->waiting_time = 0;
    p->turnaround_time = 0;
    p->quantum_remaining = sys.quantum;
    
    sys.process_count++;
    
    char log_msg[256];
    sprintf(log_msg, "Proceso PID %d creado (Burst: %d, Prioridad: %d, Memoria: %d bloques = %dMB)", 
            p->pid, burst_time, priority, memory_blocks, memory_blocks * MEMORY_BLOCK_SIZE);
    add_log(p->pid, log_msg);
    
    printf("✓ Proceso creado exitosamente (PID: %d)\n", p->pid);
    return p->pid;
}

bool check_deadlock_prevention(int pid, Resources req) {
    if (req.cpu > sys.available.cpu || req.memory_blocks > sys.available.memory_blocks) {
        return false;
    }
    return true;
}

bool request_resources(int pid, Resources req) {
    int idx = find_process_by_pid(pid);
    if (idx == -1) return false;
    
    PCB *p = &sys.processes[idx];
    
    if (!check_deadlock_prevention(pid, req)) {
        char log_msg[256];
        sprintf(log_msg, "Solicitud de recursos denegada para PID %d (prevencion deadlock)", pid);
        add_log(pid, log_msg);
        p->state = WAITING;
        return false;
    }
    
    if (req.cpu <= sys.available.cpu && req.memory_blocks <= sys.available.memory_blocks) {
        sys.available.cpu -= req.cpu;
        sys.available.memory_blocks -= req.memory_blocks;
        p->allocated.cpu += req.cpu;
        p->allocated.memory_blocks += req.memory_blocks;
        
        char log_msg[256];
        sprintf(log_msg, "Recursos asignados a PID %d (CPU: %d, Memoria: %d bloques = %dMB)", 
                pid, req.cpu, req.memory_blocks, req.memory_blocks * MEMORY_BLOCK_SIZE);
        add_log(pid, log_msg);
        return true;
    }
    
    p->state = WAITING;
    char log_msg[256];
    sprintf(log_msg, "PID %d en espera de recursos", pid);
    add_log(pid, log_msg);
    return false;
}

void release_resources(int pid) {
    int idx = find_process_by_pid(pid);
    if (idx == -1) return;
    
    PCB *p = &sys.processes[idx];
    
    sys.available.cpu += p->allocated.cpu;
    sys.available.memory_blocks += p->allocated.memory_blocks;
    
    char log_msg[256];
    sprintf(log_msg, "Recursos liberados de PID %d (CPU: %d, Memoria: %d bloques = %dMB)", 
            pid, p->allocated.cpu, p->allocated.memory_blocks, 
            p->allocated.memory_blocks * MEMORY_BLOCK_SIZE);
    add_log(pid, log_msg);
    
    p->allocated.cpu = 0;
    p->allocated.memory_blocks = 0;
}

void suspend_process(int pid) {
    int idx = find_process_by_pid(pid);
    if (idx == -1) {
        printf("Error: Proceso PID %d no encontrado\n", pid);
        return;
    }
    
    PCB *p = &sys.processes[idx];
    
    if (p->state == TERMINATED) {
        printf("Error: El proceso ya esta terminado\n");
        return;
    }
    
    if (p->state == RUNNING) {
        sys.running_pid = -1;
    }
    
    p->state = WAITING;
    char log_msg[256];
    sprintf(log_msg, "Proceso PID %d suspendido", pid);
    add_log(pid, log_msg);
    printf("✓ Proceso PID %d suspendido\n", pid);
}

void resume_process(int pid) {
    int idx = find_process_by_pid(pid);
    if (idx == -1) {
        printf("Error: Proceso PID %d no encontrado\n", pid);
        return;
    }
    
    PCB *p = &sys.processes[idx];
    
    if (p->state != WAITING) {
        printf("Error: El proceso no esta en estado WAITING\n");
        return;
    }
    
    p->state = READY;
    char log_msg[256];
    sprintf(log_msg, "Proceso PID %d reanudado", pid);
    add_log(pid, log_msg);
    printf("✓ Proceso PID %d reanudado\n", pid);
}

void terminate_process(int pid, TerminationCause cause) {
    int idx = find_process_by_pid(pid);
    if (idx == -1) {
        printf("Error: Proceso PID %d no encontrado\n", pid);
        return;
    }
    
    PCB *p = &sys.processes[idx];
    
    if (p->state == TERMINATED) {
        printf("Error: El proceso ya esta terminado\n");
        return;
    }
    
    if (p->state == RUNNING) {
        sys.running_pid = -1;
    }
    
    release_resources(pid);
    p->state = TERMINATED;
    p->termination_cause = cause;
    p->completion_time = sys.current_time;
    p->turnaround_time = p->completion_time - p->arrival_time;
    
    sys.total_processes_completed++;
    sys.total_waiting_time += p->waiting_time;
    sys.total_turnaround_time += p->turnaround_time;
    
    const char *cause_str;
    switch(cause) {
        case NORMAL: cause_str = "Normal"; break;
        case ERROR_TERMINATION: cause_str = "Error"; break;
        case DEADLOCK: cause_str = "Deadlock"; break;
        case USER_TERMINATION: cause_str = "Usuario"; break;
        default: cause_str = "Desconocida";
    }
    
    char log_msg[256];
    sprintf(log_msg, "Proceso PID %d terminado (Causa: %s, Turnaround: %d, Espera: %d)", 
            pid, cause_str, p->turnaround_time, p->waiting_time);
    add_log(pid, log_msg);
    printf("✓ Proceso PID %d terminado\n", pid);
}

int find_process_by_pid(int pid) {
    for (int i = 0; i < sys.process_count; i++) {
        if (sys.processes[i].pid == pid) {
            return i;
        }
    }
    return -1;
}

void select_next_process() {
    int selected_idx = -1;
    
    switch(sys.algorithm) {
        case FCFS:
            for (int i = 0; i < sys.process_count; i++) {
                if (sys.processes[i].state == READY) {
                    if (selected_idx == -1 || 
                        sys.processes[i].arrival_time < sys.processes[selected_idx].arrival_time) {
                        selected_idx = i;
                    }
                }
            }
            break;
            
        case ROUND_ROBIN:
            for (int i = 0; i < sys.process_count; i++) {
                if (sys.processes[i].state == READY) {
                    if (sys.processes[i].pid == last_preempted_pid && last_preempted_pid != -1) {
                        continue;
                    }
                    if (selected_idx == -1) {
                        selected_idx = i;
                    } else if (sys.processes[i].pid < sys.processes[selected_idx].pid) {
                        selected_idx = i;
                    }
                }
            }
            if (selected_idx == -1 && last_preempted_pid != -1) {
                for (int i = 0; i < sys.process_count; i++) {
                    if (sys.processes[i].state == READY && sys.processes[i].pid == last_preempted_pid) {
                        selected_idx = i;
                        break;
                    }
                }
            }
            last_preempted_pid = -1;
            break;
    }
    
    if (selected_idx != -1) {
        PCB *p = &sys.processes[selected_idx];
        
        if (p->allocated.cpu == 0) {
            if (request_resources(p->pid, p->needed)) {
                p->state = RUNNING;
                sys.running_pid = p->pid;
                
                char log_msg[256];
                sprintf(log_msg, "Proceso PID %d ahora en ejecucion", p->pid);
                add_log(p->pid, log_msg);
            }
        } else {
            p->state = RUNNING;
            sys.running_pid = p->pid;
            
            char log_msg[256];
            sprintf(log_msg, "Proceso PID %d ahora en ejecucion", p->pid);
            add_log(p->pid, log_msg);
        }
    }
}

void execute_step() {
    sys.current_time++;
    
    if (sys.running_pid != -1) {
        int idx = find_process_by_pid(sys.running_pid);
        if (idx != -1) {
            PCB *p = &sys.processes[idx];
            
            if (p->state == RUNNING) {
                p->remaining_time--;
                sys.cpu_busy_time++;
                
                if (sys.algorithm == ROUND_ROBIN) {
                    p->quantum_remaining--;
                }
                
                char log_msg[256];
                sprintf(log_msg, "PID %d ejecutando (Tiempo restante: %d)", 
                        p->pid, p->remaining_time);
                add_log(p->pid, log_msg);
                
                // Verificar quantum PRIMERO (si RR)
                if (sys.algorithm == ROUND_ROBIN && p->quantum_remaining <= 0 && p->remaining_time > 0) {
                    last_preempted_pid = p->pid;
                    
                    // IMPORTANTE: Liberar recursos cuando agota quantum
                    release_resources(p->pid);
                    
                    p->state = READY;
                    sys.running_pid = -1;
                    p->quantum_remaining = sys.quantum;
                    
                    char log_msg2[256];
                    sprintf(log_msg2, "PID %d: quantum agotado, vuelve a READY", p->pid);
                    add_log(p->pid, log_msg2);
                }
                // Luego verificar terminación
                else if (p->remaining_time <= 0) {
                    terminate_process(p->pid, NORMAL);
                    sys.running_pid = -1;
                }
            }
        }
    }
    
    for (int i = 0; i < sys.process_count; i++) {
        if (sys.processes[i].state == READY || sys.processes[i].state == WAITING) {
            sys.processes[i].waiting_time++;
        }
    }
    
    if (sys.running_pid == -1) {
        select_next_process();
    }
    
    printf("\n=== Tiempo: %d ===\n", sys.current_time);
    if (sys.running_pid != -1) {
        printf("CPU: Ejecutando PID %d\n", sys.running_pid);
    } else {
        printf("CPU: Inactiva\n");
    }
}

void list_processes() {
    printf("\n========== LISTA DE PROCESOS ==========\n");
    printf("%-5s %-10s %-8s %-10s %-8s %-10s %-12s\n", 
           "PID", "Estado", "Prior.", "Burst", "Restante", "Memoria", "Espera");
    printf("-----------------------------------------------------------------------\n");
    
    for (int i = 0; i < sys.process_count; i++) {
        PCB *p = &sys.processes[i];
        const char *state_str;
        
        switch(p->state) {
            case READY: state_str = "LISTO"; break;
            case RUNNING: state_str = "EJECUTANDO"; break;
            case WAITING: state_str = "ESPERANDO"; break;
            case TERMINATED: state_str = "TERMINADO"; break;
            default: state_str = "DESCONOCIDO";
        }
        
        printf("%-5d %-10s %-8d %-10d %-8d %-10dMB %-12d\n",
               p->pid, state_str, p->priority, p->burst_time, p->remaining_time,
               p->allocated.memory_blocks * MEMORY_BLOCK_SIZE, p->waiting_time);
    }
    printf("=======================================\n");
}

void show_resources() {
    printf("\n========== RECURSOS DEL SISTEMA ==========\n");
    printf("CPU:\n");
    printf("  Total: %d\n", sys.total.cpu);
    printf("  Disponible: %d\n", sys.available.cpu);
    printf("  En uso: %d\n", sys.total.cpu - sys.available.cpu);
    printf("\nMemoria:\n");
    printf("  Total: %d bloques (%d MB)\n", 
           sys.total.memory_blocks, sys.total.memory_blocks * MEMORY_BLOCK_SIZE);
    printf("  Disponible: %d bloques (%d MB)\n", 
           sys.available.memory_blocks, sys.available.memory_blocks * MEMORY_BLOCK_SIZE);
    printf("  En uso: %d bloques (%d MB)\n", 
           sys.total.memory_blocks - sys.available.memory_blocks,
           (sys.total.memory_blocks - sys.available.memory_blocks) * MEMORY_BLOCK_SIZE);
    printf("==========================================\n");
}

void show_logs() {
    printf("\n========== LOGS DEL SISTEMA ==========\n");
    printf("%-8s %-6s %-60s\n", "Tiempo", "PID", "Evento");
    printf("-------------------------------------------------------------------------------\n");
    
    int start = sys.log_count > 20 ? sys.log_count - 20 : 0;
    for (int i = start; i < sys.log_count; i++) {
        printf("%-8d %-6d %-60s\n", 
               sys.logs[i].time, 
               sys.logs[i].pid, 
               sys.logs[i].event);
    }
    printf("======================================\n");
}

void show_statistics() {
    printf("\n========== ESTADISTICAS DEL SISTEMA ==========\n");
    printf("Tiempo total de simulacion: %d unidades\n", sys.current_time);
    printf("Procesos completados: %d\n", sys.total_processes_completed);
    
    if (sys.total_processes_completed > 0) {
        float avg_waiting = (float)sys.total_waiting_time / sys.total_processes_completed;
        float avg_turnaround = (float)sys.total_turnaround_time / sys.total_processes_completed;
        float cpu_utilization = sys.current_time > 0 ? 
                                (float)sys.cpu_busy_time * 100 / sys.current_time : 0;
        float throughput = (float)sys.total_processes_completed / sys.current_time;
        
        printf("Tiempo promedio de espera: %.2f unidades\n", avg_waiting);
        printf("Tiempo promedio de retorno: %.2f unidades\n", avg_turnaround);
        printf("Utilizacion de CPU: %.2f%%\n", cpu_utilization);
        printf("Throughput: %.4f procesos/unidad de tiempo\n", throughput);
    }
    
    printf("\nAlgoritmo de planificacion: ");
    switch(sys.algorithm) {
        case FCFS: printf("FCFS (First Come First Served)\n"); break;
        case ROUND_ROBIN: printf("Round Robin (Quantum: %d)\n", sys.quantum); break;
    }
    
    printf("==============================================\n");
}

void send_message(int sender_pid, int receiver_pid, const char *content) {
    if (sys.message_count >= MAX_MESSAGES) {
        printf("Error: Limite de mensajes alcanzado\n");
        return;
    }
    
    Message *msg = &sys.messages[sys.message_count++];
    msg->sender_pid = sender_pid;
    msg->receiver_pid = receiver_pid;
    strncpy(msg->content, content, 255);
    msg->content[255] = '\0';
    msg->delivered = false;
    
    char log_msg[256];
    sprintf(log_msg, "Mensaje enviado de PID %d a PID %d", sender_pid, receiver_pid);
    add_log(sender_pid, log_msg);
    
    printf("✓ Mensaje enviado\n");
}

void receive_message(int receiver_pid) {
    bool found = false;
    
    printf("\n=== Mensajes para PID %d ===\n", receiver_pid);
    for (int i = 0; i < sys.message_count; i++) {
        if (sys.messages[i].receiver_pid == receiver_pid && !sys.messages[i].delivered) {
            printf("De PID %d: %s\n", sys.messages[i].sender_pid, sys.messages[i].content);
            sys.messages[i].delivered = true;
            found = true;
            
            char log_msg[256];
            sprintf(log_msg, "Mensaje recibido por PID %d de PID %d", 
                    receiver_pid, sys.messages[i].sender_pid);
            add_log(receiver_pid, log_msg);
        }
    }
    
    if (!found) {
        printf("No hay mensajes nuevos\n");
    }
    printf("============================\n");
}

int create_semaphore(int initial_value) {
    if (sys.semaphore_count >= MAX_SEMAPHORES) {
        printf("Error: Limite de semaforos alcanzado\n");
        return -1;
    }
    
    int id = sys.semaphore_count;
    Semaphore *sem = &sys.semaphores[id];
    sem->id = id;
    sem->value = initial_value;
    sem->waiting_count = 0;
    
    sys.semaphore_count++;
    
    char log_msg[256];
    sprintf(log_msg, "Semaforo %d creado con valor inicial %d", id, initial_value);
    add_log(-1, log_msg);
    
    return id;
}

void wait_semaphore(int sem_id, int pid) {
    if (sem_id < 0 || sem_id >= sys.semaphore_count) {
        printf("Error: Semaforo invalido\n");
        return;
    }
    
    Semaphore *sem = &sys.semaphores[sem_id];
    sem->value--;
    
    char log_msg[256];
    sprintf(log_msg, "PID %d ejecuta wait en semaforo %d (valor: %d)", 
            pid, sem_id, sem->value);
    add_log(pid, log_msg);
    
    if (sem->value < 0) {
        int idx = find_process_by_pid(pid);
        if (idx != -1) {
            sys.processes[idx].state = WAITING;
            sem->waiting_pids[sem->waiting_count++] = pid;
            
            sprintf(log_msg, "PID %d bloqueado esperando semaforo %d", pid, sem_id);
            add_log(pid, log_msg);
        }
    }
}

void signal_semaphore(int sem_id) {
    if (sem_id < 0 || sem_id >= sys.semaphore_count) {
        printf("Error: Semaforo invalido\n");
        return;
    }
    
    Semaphore *sem = &sys.semaphores[sem_id];
    sem->value++;
    
    char log_msg[256];
    sprintf(log_msg, "Signal ejecutado en semaforo %d (valor: %d)", sem_id, sem->value);
    add_log(-1, log_msg);
    
    if (sem->value <= 0 && sem->waiting_count > 0) {
        int pid = sem->waiting_pids[0];
        
        for (int i = 0; i < sem->waiting_count - 1; i++) {
            sem->waiting_pids[i] = sem->waiting_pids[i + 1];
        }
        sem->waiting_count--;
        
        int idx = find_process_by_pid(pid);
        if (idx != -1) {
            sys.processes[idx].state = READY;
            
            sprintf(log_msg, "PID %d desbloqueado del semaforo %d", pid, sem_id);
            add_log(pid, log_msg);
        }
    }
}

void demonstrate_producer_consumer() {
    printf("\n========== DEMOSTRACION PRODUCTOR-CONSUMIDOR ==========\n");
    printf("Configuracion: 2 Productores, 2 Consumidores, Buffer de tamano %d\n\n", BUFFER_SIZE);
    
    sys.mutex = create_semaphore(1);
    sys.empty = create_semaphore(BUFFER_SIZE);
    sys.full = create_semaphore(0);
    
    printf("Semaforos creados:\n");
    printf("  - mutex (ID: %d): control de acceso al buffer\n", sys.mutex);
    printf("  - empty (ID: %d): espacios vacios en buffer\n", sys.empty);
    printf("  - full (ID: %d): espacios llenos en buffer\n", sys.full);
    
    int prod1 = create_process(10, 5, 1);
    printf("Productor 1 creado (PID: %d)\n", prod1);
    
    int prod2 = create_process(10, 5, 1);
    printf("Productor 2 creado (PID: %d)\n", prod2);
    
    int cons1 = create_process(10, 5, 1);
    printf("Consumidor 1 creado (PID: %d)\n", cons1);
    
    int cons2 = create_process(10, 5, 1);
    printf("Consumidor 2 creado (PID: %d)\n", cons2);
    
    printf("\nLos procesos productores y consumidores han sido creados.\n");
    printf("Use 'Ejecutar siguiente paso' para simular la produccion/consumo.\n");
    printf("=======================================================\n");
}

void select_algorithm() {
    printf("\n========== SELECCION DE ALGORITMO DE PLANIFICACION ==========\n");
    printf("1. FCFS (First Come First Served)\n");
    printf("2. Round Robin\n");
    printf("Seleccione un algoritmo (1-2): ");
    
    int choice;
    scanf("%d", &choice);
    
    switch(choice) {
        case 1:
            sys.algorithm = FCFS;
            printf("✓ Algoritmo FCFS seleccionado\n");
            add_log(-1, "Algoritmo FCFS seleccionado");
            break;
        case 2:
            sys.algorithm = ROUND_ROBIN;
            printf("Ingrese el quantum: ");
            scanf("%d", &sys.quantum);
            printf("✓ Algoritmo Round Robin seleccionado (Quantum: %d)\n", sys.quantum);
            char log_msg[256];
            sprintf(log_msg, "Algoritmo Round Robin seleccionado (Quantum: %d)", sys.quantum);
            add_log(-1, log_msg);
            break;
        default:
            printf("Opcion invalida. Se selecciono FCFS por defecto.\n");
            sys.algorithm = FCFS;
    }
    printf("============================================================\n");
}

void show_menu() {
    printf("\n");
    printf("============================================================\n");
    printf("     SIMULADOR DE GESTOR DE PROCESOS - MENU PRINCIPAL\n");
    printf("============================================================\n");
    printf("  1. Crear proceso\n");
    printf("  2. Listar procesos activos\n");
    printf("  3. Mostrar recursos disponibles\n");
    printf("  4. Ejecutar siguiente paso\n");
    printf("  5. Suspender proceso\n");
    printf("  6. Reanudar proceso\n");
    printf("  7. Terminar proceso\n");
    printf("  8. Ver logs\n");
    printf("  9. Ver estadisticas\n");
    printf(" 10. Enviar mensaje\n");
    printf(" 11. Recibir mensajes\n");
    printf(" 12. Crear semaforo\n");
    printf(" 13. Wait en semaforo\n");
    printf(" 14. Signal en semaforo\n");
    printf(" 15. Demo Productor-Consumidor\n");
    printf("  0. Salir\n");
    printf("============================================================\n");
    printf("Seleccione una opcion: ");
}

int main() {
    printf("===============================================================\n");
    printf("  SIMULADOR DE GESTOR DE PROCESOS PARA SISTEMAS OPERATIVOS\n");
    printf("           Universidad Autonoma de Tamaulipas\n");
    printf("===============================================================\n\n");
    
    init_system();
    select_algorithm();
    
    int option;
    int pid, burst_time, priority, memory_blocks;
    int sender_pid, receiver_pid;
    char message_content[256];
    int sem_id, initial_value;
    
    while (1) {
        show_menu();
        scanf("%d", &option);
        
        switch(option) {
            case 1:
                printf("\n--- Crear Proceso ---\n");
                printf("Tiempo de ejecucion (burst time): ");
                scanf("%d", &burst_time);
                printf("Prioridad (mayor numero = mayor prioridad): ");
                scanf("%d", &priority);
                printf("Bloques de memoria necesarios (1 bloque = %dMB, max %d bloques): ", 
                       MEMORY_BLOCK_SIZE, MAX_MEMORY_BLOCKS);
                scanf("%d", &memory_blocks);
                create_process(burst_time, priority, memory_blocks);
                break;
                
            case 2:
                list_processes();
                break;
                
            case 3:
                show_resources();
                break;
                
            case 4:
                execute_step();
                break;
                
            case 5:
                printf("\n--- Suspender Proceso ---\n");
                printf("PID del proceso a suspender: ");
                scanf("%d", &pid);
                suspend_process(pid);
                break;
                
            case 6:
                printf("\n--- Reanudar Proceso ---\n");
                printf("PID del proceso a reanudar: ");
                scanf("%d", &pid);
                resume_process(pid);
                break;
                
            case 7:
                printf("\n--- Terminar Proceso ---\n");
                printf("PID del proceso a terminar: ");
                scanf("%d", &pid);
                terminate_process(pid, USER_TERMINATION);
                break;
                
            case 8:
                show_logs();
                break;
                
            case 9:
                show_statistics();
                break;
                
            case 10:
                printf("\n--- Enviar Mensaje ---\n");
                printf("PID emisor: ");
                scanf("%d", &sender_pid);
                printf("PID receptor: ");
                scanf("%d", &receiver_pid);
                printf("Contenido del mensaje: ");
                getchar(); // limpiar buffer
                fgets(message_content, 256, stdin);
                message_content[strcspn(message_content, "\n")] = 0; // remover newline
                send_message(sender_pid, receiver_pid, message_content);
                break;
                
            case 11:
                printf("\n--- Recibir Mensajes ---\n");
                printf("PID receptor: ");
                scanf("%d", &receiver_pid);
                receive_message(receiver_pid);
                break;
                
            case 12:
                printf("\n--- Crear Semaforo ---\n");
                printf("Valor inicial: ");
                scanf("%d", &initial_value);
                sem_id = create_semaphore(initial_value);
                if (sem_id != -1) {
                    printf("✓ Semaforo creado con ID: %d\n", sem_id);
                }
                break;
                
            case 13:
                printf("\n--- Wait en Semaforo ---\n");
                printf("ID del semaforo: ");
                scanf("%d", &sem_id);
                printf("PID del proceso: ");
                scanf("%d", &pid);
                wait_semaphore(sem_id, pid);
                printf("✓ Wait ejecutado\n");
                break;
                
            case 14:
                printf("\n--- Signal en Semaforo ---\n");
                printf("ID del semaforo: ");
                scanf("%d", &sem_id);
                signal_semaphore(sem_id);
                printf("✓ Signal ejecutado\n");
                break;
                
            case 15:
                demonstrate_producer_consumer();
                break;
                
            case 0:
                printf("\n¡Gracias por usar el simulador!\n");
                printf("Estadisticas finales:\n");
                show_statistics();
                return 0;
                
            default:
                printf("Opcion invalida. Intente de nuevo.\n");
        }
    }
    
    return 0;
}