/*
 * File: threading.c
 * ------------------
 * Thread manager functions. 
 */

#include "gl.h"
#include "timer.h"
#include "printf.h"
#include "malloc.h"
#include "armtimer.h"
#include "register.h"
#include "threading.h"
#include "threading_asm.h"

extern unsigned __text_end__;
extern unsigned __thread_end__;

extern unsigned __thread_start__;
const  unsigned thread_start = (unsigned) &__thread_start__;

volatile unsigned interrupt_pc; 
volatile thread_manager_t* manager;

static wait_event w;

/*
 * Function: rand
 * ----------------
 * Medium quality random number generator. 
 */
#define INT_MAX 4294967295
int rand() {
    unsigned seed = timer_get_time();
    static long state = 3;

    state = (((state * 214013 + 2531011) >> 16) & 32767);
    state *= seed & 0xFF;

    return ((state % INT_MAX) + 1);
}

/*
* Function: set_priority
* ----------------------
* Changes the priority of the current thread
*/
void set_priority(int priority){
    CURRENT_THREAD.priority = priority;
}

/*
 * Function: find_next_thread
 * ---------------------------
 * Index for next thread that shuld run. 
 */
int find_next_thread() {
    int next_thread = manager->current_thread;
    int iterations = 0;

    while (1) {
        // check out the next thread in the queue
        next_thread = (next_thread + 1) % manager->thread_counter;
        volatile thread_t * candidate = &manager->threads[next_thread];
        thread_status status = candidate->status;


        // valid thread that can run
        if (status == ACTIVE || status == INACTIVE) {
            // only go to a lesser priority if you can't find a higher or equal one
            if (candidate->priority >= CURRENT_THREAD.priority || iterations) {
                break;
            }
        }

        // thread had been paused for a time, but that time has passed
        if (status == WAITING_TIME && candidate->wait_status.end_time > timer_get_time()) {
            break;
        }

        // thread had been waiting for a semaphore to have been set, and it now is
        if (status == WAITING_SEM && get_sem(candidate->wait_status.sem_id)) {
            set_sem(CURRENT_THREAD.wait_status.sem_id, 0);
            break;
        }

        if (next_thread == CURRENT_THREAD.id){
            iterations ++;
        }
    }

    return next_thread;
}

/*
 * Function: wait_sem
 * --------------------
 * Wait for semaphore. 
 */
void wait_sem(unsigned id) {
    CURRENT_THREAD.status = WAITING_SEM;
    CURRENT_THREAD.wait_status.sem_id = id;
    yield();
}

/*
 * Function: task_delay
 * --------------------
 * Tells the current thread to wait for (at least) us microseconds. 
 * Used asynchronously. 
 */
void task_delay(unsigned us) {
    RELOAD_CONTROLLER;
    CURRENT_THREAD.status = WAITING_TIME;
    CURRENT_THREAD.wait_status.end_time = timer_get_time() + us;
    yield();
}

/*
 * Function: get_sem
 * --------------------
 * Returns the value of the semaphore with the given id
 */
unsigned get_sem(unsigned id) {
    return *(SEMAPHORE_START + id);
}

/*
 * Function: set_sem
 * --------------------
 * Sets the value of the semaphore with the given id
 */
void set_sem(unsigned id, unsigned short val) {
    *(SEMAPHORE_START + id) = val;
}

/*
 * Function: create_sem
 * ----------------------
 * Creates a semaphore
 */
void create_sem(unsigned id) {
    // empty
}

/*
 * Function: check_registers
 * --------------------------
 * Checks register during a timer context switch. 
 */
void check_registers() {
    unsigned sp, lr, r1;
    __asm__("mov %0, r0" : "=r" (lr));
    __asm__("mov %0, sp" : "=r" (sp));
    __asm__("mov %0, r1" : "=r" (r1));
    safe_printf("lr = 0x%08x | sp = 0x%08x | r1 = 0x%08x | saved lr = 0x%08x | saved r1 = 0x%08x \n", lr, sp, r1, CURRENT_THREAD.lr, CURRENT_THREAD.r1);
}

/*
 * Function: increment_thread
 * ---------------------------
 * Goes to the next thread and sets r0 to the new address. 
 */
void increment_thread() {
    CURRENT_THREAD.pc = interrupt_pc; 
    manager->current_thread = find_next_thread();
    armtimer_clear_interrupt(); 
    __asm__ volatile("mov r0,  %0" : : "r" ((unsigned) &(CURRENT_THREAD)));
}


/*
 * Function: increment_thread
 * ---------------------------
 * Goes to the next thread and sets r0 to the new address.
 */
void increment_thread_cooperative() {
    int old_thread = manager->current_thread;
    manager->current_thread = find_next_thread();
    safe_printf("----- Switched from thread %i to thread %i \n", old_thread, manager->current_thread);

    __asm__ volatile("mov r0,  %0" : : "r" ((unsigned) &(CURRENT_THREAD)));
}

/*
 * Function: get_current_thread
 * -----------------------------
 * Sets r0 to the current thread address. 
 */
void get_current_thread() {
    __asm__ volatile("mov r0,  %0" : : "r" ((unsigned) &(CURRENT_THREAD)));
}

/*
 * Function: find_next_thread_coop
 * --------------------------------
 * Switch to next thread for cooperative yielding. 
 */
int find_next_thread_coop() {
    int next_thread = manager->current_thread;
    int iterations = 0;

    while (1) {
        //check out the next thread in the queue
        next_thread = (next_thread + 1) % manager->thread_counter;

        volatile thread_t* candidate = &manager->threads[next_thread];
        thread_status status = candidate->status;


        // valid thread that can run
        if (status == ACTIVE || status == INACTIVE) {
            //only go to a lesser priority if you can't find a higher or equal one
            if (candidate->priority >= CURRENT_THREAD.priority || iterations) {
                break;
            }
        }

        // thread had been paused for a time, but that time has passed
        if (status == WAITING_TIME && candidate->wait_status.end_time > timer_get_time()) {
            break;
        }

        // thread had been waiting for a semaphore to have been set, and it now is
        if (status == WAITING_SEM && get_sem(candidate->wait_status.sem_id)) {
            set_sem(CURRENT_THREAD.wait_status.sem_id, 0);
            break;
        }

        if (next_thread == CURRENT_THREAD.id){
            iterations ++;
        }
    }

    return next_thread;
}

static unsigned yield_counter = 0; 

/*
 * Function: yield
 * -----------------
 * Chooses the next thread to switch to and yields to that
 * NOTE: This should only be used asynchronously. 
 */
void yield() {
    RELOAD_CONTROLLER;
    // save the current registers
    PUSH_ALL;
    unsigned sp;
    __asm__("mov %0, sp" : "=r"(sp));
    manager->threads[manager->current_thread].sp = sp;

    // switch to next thread
    int next_thread = (manager->current_thread + 1) % manager->thread_counter; 
    yield_counter ++; 
    if (yield_counter % 1000 == 0) {
        printf("%u yields have occurred.\n", yield_counter);
    }

    if (CURRENT_THREAD.status == ACTIVE) {
        CURRENT_THREAD.status = INACTIVE;
    }
    manager->current_thread = next_thread;

    if (CURRENT_THREAD.status == WAITING_SEM) {
        set_sem(CURRENT_THREAD.wait_status.sem_id, 0);
    }
    CURRENT_THREAD.status = ACTIVE;

    // reload the variables from that thread
    SET_SP(CURRENT_THREAD.sp);
    POP_ALL;

}

/* 
 * Function: run_thread_coop
 * --------------------------
 * Create a non-preemptive thread. 
 */
unsigned sp;
void run_thread_coop(void (* func)(va_list), int priority, ... ) {
    RELOAD_CONTROLLER;

    if (manager->thread_counter == MAX_THREADS){ // don't create too many threads
        return;
    }
    safe_printf("Creating thread %d\n", manager->thread_counter);

    va_list args;
    va_start(args, priority);

    // create and store the thread object
    thread_t thread; 
    thread.run = func; 
    thread.sp = THREAD_START - THREAD_ALLOC_SIZE * manager->thread_counter; 
    thread.status = ACTIVE; 
    thread.wait_status = w; 
    thread.args = args; 
    thread.id = manager->thread_counter; 

    manager->threads[thread.id] = thread;

    // remember where *this* thread is so you can return after the initialized one is complete
    __asm__("mov %0, sp" : "=r"(sp));

    // initialize that thread's stack
    SET_SP(thread.sp);
    PUSH_ALL;
    unsigned thread_sp;
    __asm__("mov %0, sp" : "=r"(thread_sp));
    RELOAD_CONTROLLER;
    manager->threads[manager->thread_counter].sp = thread_sp;

    // tell it that another thread has been created
    manager->thread_counter ++;

    thread.run(thread.args);

    // You get to here when the thread finishes
    SET_SP(sp);
    va_end(args);

    RELOAD_CONTROLLER;
    CURRENT_THREAD.status = FINISHED;
    printf("Thread %i has finished.\n", CURRENT_THREAD.id);

    while (1) yield();//don't pop the context off the stack. It's hacky but oh well
}

/*
 * Function: run_thread
 * --------------------
 * Create and run thread. Used for preemptive/cooperative threading. 
 */
void run_thread(void (* func)(va_list), int priority, ... ) {
    system_disable_interrupts(); 
    RELOAD_CONTROLLER;

    if (manager->thread_counter == MAX_THREADS) return;

    safe_printf("Creating thread %d\n", manager->thread_counter);
    safe_printf("New SP = %08x\n", thread_start + THREAD_ALLOC_SIZE * (manager->thread_counter + 1));

    w.end_time = 0;

    thread_t thread;
    thread.run = func; 
    va_list args;
    va_start(args, priority);

    thread.r0 = thread.r1 = thread.r2 = thread.r3 = thread.r4 = thread.r5 = thread.r6 = thread.r7 = thread.r8 = 0; 
    thread.r8 = thread.r9 = thread.r10 = thread.r11 = thread.r12 = 0; 

    thread.lr = 0; 
    safe_printf("func = %08x\n", func); 
    safe_printf("thread lr = %08x\n", thread.lr);

    thread.pc   = (unsigned) func;
    thread.sp   = thread_start + THREAD_ALLOC_SIZE * (manager->thread_counter + 1);
    thread.id   = manager->thread_counter;
    thread.cpsr = GETCPSR();
    thread.args = args; 
    thread.status = ACTIVE;
    thread.priority = priority;
    thread.wait_status = w;

    if (manager->current_thread) {
        CURRENT_THREAD.status = INACTIVE;
    }
    manager->threads[thread.id] = thread;
    manager->current_thread = thread.id; 
    manager->thread_counter ++;

    SET_SP(thread.sp);
    va_end(args);

    system_enable_interrupts(); 
    thread.run(thread.args);

    RELOAD_CONTROLLER;
    CURRENT_THREAD.status = FINISHED;

    safe_printf("Thread %i has finished.\n", CURRENT_THREAD.id);

    while (1) yield(); // TODO
}

/*
 * Function: thread_manager_init
 * ------------------------------
 * Handles all the initialization for threads; essentially just zeros out the memory. 
 */
void thread_manager_init(unsigned mode) {
    RELOAD_CONTROLLER;
    manager->current_thread = 0;
    manager->thread_counter = 0;

    timer_init();

    if (mode) {
        armtimer_init(); 
    }
}

/*
 * Function: memcpy
 * -----------------
 * Simple implementation of memcpy (online)
 */
void memcpy(void *dest, void *src, unsigned n) {
    char *csrc = (char *)src;
    char *cdest = (char *)dest;

    for (int i=0; i<n; i++)
        cdest[i] = csrc[i];
}

/*
 * Function: memset
 * -----------------
 * Simple implementation of memset (online)
 */
void *memset(void *b, int c, int len) {
    unsigned char *p = b;
    while(len > 0) {
        *p = c;
        p ++;
        len --;
    }
    return(b);
}

/*
 * Function: time_delay
 * ---------------------
 * Simplified delay function that calls nothing else. 
 */
void time_delay(unsigned n) {
    volatile unsigned start = *((volatile unsigned *) 0x20007004); 
    while (*((volatile unsigned *) 0x20007004) - start < n); 
}

/*
 * Function: pastel
 * ---------------------
 * Returns a pleasing pastel color. 
 * See http://stackoverflow.com/questions/43044/algorithm-to-
 * randomly-generate-an-aesthetically-pleasing-color-palette
 */
unsigned pastel() {
    int red = rand() % 256;
    int green = rand() % 256; 
    int blue = rand() % 256; 

    red = (red + 0xFF) / 2;
    green = (green + 0xFF) / 2;
    blue = (blue + 0xFF) / 2;
    
    return gl_color(red, green, blue);
}