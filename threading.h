/*
 * File: threading.h
 * ----------------
 * Definitions for task creation and management. 
 */

#ifndef THREADING_H
#define THREADING_H

#include <stdarg.h>

extern unsigned __thread_start__; 
extern unsigned __thread_manager__; 

#define MAX_THREADS         16
#define THREAD_START        0x700000
#define THREAD_ALLOC_SIZE   0x8000
#define SEMAPHORE_START     (unsigned *) 0x300000 // Each semaphore is one 'word'

#define SET_SP(sp_var) __asm__("mov sp, %0" : : "r" (sp_var));
#define PUSH_ALL __asm__("push { r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r14 }");
#define POP_ALL __asm__("pop { r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r14 }");
#define RELOAD_CONTROLLER   manager = (thread_manager_t *) &__thread_manager__;

// #define YIELD __asm__("push {r0-r12, lr}"); __asm__("bl yield"); __asm__("pop {r0-r12, lr}");

#define CURRENT_THREAD manager->threads[manager->current_thread]

extern int (*safe_printf)(const char * format, ...);

// Stores information for a waiting thread
typedef union {
    unsigned end_time;
    unsigned sem_id;
} wait_event;

// States that a thread can be in at a given time
typedef enum __attribute__((packed, aligned(8))) {
    ACTIVE,
    INACTIVE,
    WAITING_SEM,
    WAITING_TIME,
    TERMINATED,
    FINISHED
} thread_status;

/*
 * Struct: thread_t 
 * -----------------
 * All data required to store a thread's context. 
 */
typedef struct __attribute__((packed, aligned(8))) {
    void (*run)();
    unsigned r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, sp, lr, pc;
    unsigned cpsr, spsr;
    unsigned id;
    unsigned priority;
    thread_status status;
    wait_event wait_status;
    va_list args;
} thread_t;

/*
 * Struct: thread_manager_t 
 * -------------------------
 * Data structure to manage threads. 
 */
typedef struct {
    unsigned current_thread;
    unsigned thread_counter;
    thread_t threads[MAX_THREADS];
} thread_manager_t;

// Thread management functions 
void yield();
int find_next_thread();
void task_delay(unsigned time);
void thread_manager_init(unsigned mode);
void run_thread(void (* func)(va_list), int priority, ... ); 
void run_thread_coop(void (* func)(va_list), int priority, ... ); 

void increment_thread_cooperative();
void set_priority(int priority);

// Semaphore Functions
void wait_sem(unsigned id);
void create_sem(unsigned id);
unsigned get_sem(unsigned id);
void set_sem(unsigned id, unsigned short val);

// Helper functions
int rand(); 
unsigned pastel(); 
void time_delay(unsigned n); 
void *memset(void *b, int c, int len);
void memcpy(void *dest, void *src, unsigned n); 

#endif