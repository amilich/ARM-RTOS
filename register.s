.globl SETCPSR
SETCPSR:
    msr cpsr, r0
    bx lr

.globl SETSPSR
SETSPSR:
    msr spsr, r0
    bx lr

.globl GETCPSR
GETCPSR:
    mrs r0, cpsr
    bx lr

.globl GETSPSR
GETSPSR:
    mrs r0, spsr
    bx lr


/*
 * Enable/disable interrupts.
 *
 * CPSR = current program status register
 *        upper bits are differen carry flags.
 *        lower 8:
 *          7 6 5 4 3 2 1 0
 *          +-+-+-+---------+
 *          |I|F|T|   Mode  |
 *          +-+-+-+---------+
 *
 *  I : disables IRQ when = 1.
 *  F : disables FIQ when = 1.
 *  T : = 0 indicates ARM execution, = 1 is thumb execution.
 *      Mode = current mode.
 */

.global system_enable_interrupts
system_enable_interrupts:
    push {r0}
    mrs r0,cpsr
    bic r0,r0,#0x80 // I=0 enables interrupts
    msr cpsr_c,r0
    pop {r0}
    bx lr

.global system_disable_interrupts
system_disable_interrupts:
    push {r0}
    mrs r0,cpsr
    orr r0,r0,#0x80 // I=1 disables interrupts
    msr cpsr_c,r0
    pop {r0}
    bx lr
