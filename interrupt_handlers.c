/* 
 * File: interrupt_handlers.c 
 * ---------------------------
 * Interrupt handlers. 
 */

#include "gpio.h"
#include "gprof.h"
#include "mouse.h"
#include "timer.h"
#include "printf.h"
#include "armtimer.h"
#include "keyboard.h"
#include "register.h"
#include "threading.h"
#include "gpioevent.h"

extern void keyboard_vector();
extern void mouse_vector();
extern thread_manager_t* manager;

extern unsigned interrupt_pc; 

static unsigned last_time = 0; 

unsigned context_switch_count = 0; 

/* 
 * Function: interrupt_vector
 * --------------------------
 * Manages interrupt vectors.  
 */
void interrupt_vector (unsigned pc) {
	interrupt_pc = pc; 
	system_disable_interrupts(); 

    if (armtimer_has_interrupt()) {
    	context_switch_count ++; 
    	armtimer_clear_interrupt();
    } else if (gpio_check_and_clear_event(21)) {
    	printf("Manual GPIO yield\n");
    	delay(1); 
    	last_time = timer_get_time(); 
    } else {
    	printf("Undefined interrupt\n"); 
    }
}

void reset_vector(int pc) {}
void fast_interrupt_vector(int pc) {}
void undefined_instruction_vector(int pc) {}
void software_interrupt_vector(int pc) {}
void prefetch_abort_vector(int pc) {}
void data_abort_vector(int pc) {}
