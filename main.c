/*
 * File: main.c
 * -------------
 * Main. 
 * 
 * Created by Kai Marshland and Andrew Milich in Spring 2016. 
 * Tested on a Raspberry Pi 1. 
 */

#include "gl.h"
#include "gpio.h"
#include "gprof.h"
#include "timer.h"
#include "printf.h"
#include "shapes.h"
#include "system.h"
#include "windows.h"
#include "drawings.h"
#include "armtimer.h"
#include "register.h"
#include "gpioextra.h"
#include "threading.h"
#include "gpioevent.h"
#include "interrupts.h"

#define NO_GPIO 
#define COOPERATIVE       1
#define DEMO_SELECT       1

#define INTERRUPT_BUTTON  GPIO_PIN21

unsigned width  = 300;
unsigned height = 300; 

extern int __heap_start__;

static int max = 10; 
static unsigned sp, lr; 
static unsigned init_counter = 0;

int (*safe_printf)(const char * format, ...);

/*
 * Printf used for debugging. 
 */
int no_printf(const char *format, ...) {
    return 0;
}

/*
 * Function: task_init
 * --------------------
 * Preemptive demo initialization. 
 */
void task_init() {
    __asm__ volatile("mov %0, sp" : "=r"(sp));
    safe_printf("Real main starting sp = %08x count = %i\n", sp, init_counter);

    if (init_counter == 0) {
        armtimer_init();
        armtimer_start(0x10);
        system_enable_interrupts();
    }

    if (DEMO_SELECT == 0) { // rectangle demo
        for (int ii = 0; ii < max; ii ++) {
            if (init_counter == ii) {
                init_counter ++;
                run_thread(rect_thread, 1, rand() % width, rand() % height,
                           rand() % MAX_RECT_WIDTH, rand() % MAX_RECT_HEIGHT, pastel());
            }
        }
    } else if (DEMO_SELECT == 1) { // image demo
        for (int ii = 0; ii < 9; ii ++) {
            if (init_counter == ii) {
                init_counter ++;
                if (ii < 5) run_thread(image_thread, 1, 2, ii);
                else run_thread(image_thread, 1, 2, ii);
            }
        }
    }

    while (1) {
        __asm__ volatile("mov %0, sp" : "=r"(sp));
        __asm__ volatile("mov %0, lr" : "=r"(lr));
        safe_printf("In func main | my counter = %08x | sp = %08x | lr = %08x |"
                    "count addr = %08x | gpio 21 = %i.\n", init_counter ++, sp, lr, &init_counter, gpio_read(INTERRUPT_BUTTON));
    }
}

/*
 * Function: task_init_shapes
 * --------------------------- 
 * Cooperative demo. 
 */
void task_init_shapes() {
    __asm__("mov %0, sp" : "=r"(sp));

    printf("Start sp = %08x\n", sp);

    run_thread_coop(window_snakes, 2);
    run_thread_coop(window_bounce, 2);

    while (1) {
        yield(); // Run the shape threads infinitely 
    }
}

/*
 * Function: main 
 * ---------------
 * Initialize and run demo. 
 */
void main(void) {
    delay(3);

    safe_printf = no_printf;
    safe_printf("Initialized. Heap starts at 0x%08x.\n", &__heap_start__);

    if (COOPERATIVE) {
        system_enable_cache(); 
        windows_init(600, 600, GL_SINGLEBUFFER);
        thread_manager_init(0);
        run_thread_coop(task_init_shapes, 1); 
    } else {
        gl_init(width, height, GL_SINGLEBUFFER);
        thread_manager_init(1);
        run_thread(task_init, 1); 
    }
}