/*
 * File: main.c
 * -------------
 * Main. 
 */

#include "threading.h"
#include "register.h"
#include "armtimer.h"
#include "printf.h"
#include "system.h"
#include "timer.h"
#include "gprof.h"
#include "gl.h"
#include "windows.h"
#include "shell.h"

void function0(va_list args);
void function1();
void function2();
void function3();

#define SEM_SWAPBUFFER    3
#define SEM_DRAW_1        4
#define SEM_DRAW_2        5

static unsigned sp, lr;

int (*safe_printf)(const char * format, ...);
int no_printf(const char *format, ...) {
    return 0;
}

void func_1() {
    unsigned counter = 0; 
    while (1) {
        __asm__("mov %0, sp" : "=r"(sp));
        __asm__("mov %0, lr" : "=r"(lr));
        safe_printf("In func 0001 | my counter = %08x | sp = %08x | lr = %08x | count addr = %08x.\n", counter ++, sp, lr, &counter);
        // safe_printf("Time = %08x\n", timer_get_time()); 
        task_delay(0x100000);
    }
}

void func_2() {
    unsigned counter = 0; 
    while (1) {
        __asm__("mov %0, sp" : "=r"(sp));
        __asm__("mov %0, lr" : "=r"(lr));
        safe_printf("In func 0002 | my counter = %08x | sp = %08x | lr = %08x | count addr = %08x.\n", counter ++, sp, lr, &counter);
        // safe_printf("Time = %08x\n", timer_get_time()); 
        task_delay(0x100000);
    }
}

/*
 * Thread based entry point into our program
 */
void real_main() {
    run_thread(func_1, 1);
    run_thread(func_2, 1);

    unsigned counter = 0; 

    while (1) {
        __asm__("mov %0, sp" : "=r"(sp));
        __asm__("mov %0, lr" : "=r"(lr));
        safe_printf("In func main | my counter = %08x | sp = %08x | lr = %08x | count addr = %08x.\n", counter ++, sp, lr, &counter);
        // safe_printf("Time = %08x\n", timer_get_time()); 
        // delay(1); 
        task_delay(0x500000);
    }
}

extern int __heap_start__; 

void main(void) {
    safe_printf = printf;
    thread_manager_init();
    timer_init();

    delay(3);

    safe_printf("Program Initialized. Heap starts at 0x%08x.\n", &__heap_start__);
    run_thread(real_main, 1);
}