/* 
 * File: vectors.s
 * -----------------
 * RPi interrupt vectors. 
 */

.globl _vectors
.globl _vectors_end
        
_vectors:
  ldr pc, _reset_asm
  ldr pc, _undefined_instruction_asm
  ldr pc, _software_interrupt_asm
  ldr pc, _prefetch_abort_asm
  ldr pc, _data_abort_asm
  ldr pc, _reset_asm
  ldr pc, _interrupt_asm
fast_interrupt_asm:
  sub   lr, lr, #4 @First instr of FIQ handler
  push  {lr}
  push  {r0-r12}
  mov   r0, lr              @ Pass old pc
  bl    fast_interrupt_vector    @ C function
  pop   {r0-r12}
  ldm   sp!, {pc}^

_reset_asm:                   .word reset_asm
_undefined_instruction_asm:   .word undefined_instruction_asm
_software_interrupt_asm:      .word software_interrupt_asm
_prefetch_abort_asm:          .word prefetch_abort_asm
_data_abort_asm:              .word data_abort_asm
_interrupt_asm:               .word interrupt_asm

_vectors_end:

/* 
 * interrupt_asm
 * --------------
 * This will run for both timer and GPIO interrupts. 
 */
interrupt_asm:
    //store basic interrupt stuff
    sub   lr, lr, #4
    push  { r0-r12, lr}
    bl    system_disable_interrupts 
    pop   { r0-r12, lr}

    //call the interrupt vector 
    push  { r0-r12 }
    mov   r0, lr            
    bl    interrupt_vector  
    pop   { r0-r12 }

    save_current_thread:
        //remember r1 so you can use it for r0
        push {r1}
        mov r1, r0 //store r0 so it can be restored

        push {r2, r3}
        bl get_current_thread //r0 now has the address of CURRENT_THREAD
        pop {r2, r3}

        add r0, r0, #4 // r0 = &CURRENT_THREAD.r0
        str r1, [r0]   // save what the r0 was

        pop {r1}          // restore r1
        add r0, r0, #4   // r0 = &CURRENT_THREAD.r1
        str r1, [r0]    // save r1

        //r2
        add r0, r0, #4
        str r2, [r0]

        //r3
        add r0, r0, #4
        str r3, [r0]

        //r4
        add r0, r0, #4
        str r4, [r0]

        //r5
        add r0, r0, #4
        str r5, [r0]

        //r6
        add r0, r0, #4
        str r6, [r0]

        //r7
        add r0, r0, #4
        str r7, [r0]

        //r8
        add r0, r0, #4
        str r8, [r0]

        //r9
        add r0, r0, #4
        str r9, [r0]

        //r10
        add r0, r0, #4
        str r10, [r0]

        //r11
        add r0, r0, #4
        str r11, [r0]

        //r12
        add r0, r0, #4
        str r12, [r0]

        //store SVC sp, lr, and pc
        mrs r1, cpsr
        bic r1, r1, #0x1F
        orr r1, r1, #0x13
        msr cpsr_c, r1

        //sp
        add r0, r0, #4
        str sp, [r0]

        //lr
        add r0, r0, #4
        str lr, [r0]

        //back to IRQ land
        mrs r1, cpsr
        bic r1, r1, #0x1F
        orr r1, r1, #0x12
        msr cpsr_c, r1
 
        //pc THIS NEEDS TO BE LR 
        add r0, r0, #4
        str lr, [r0]

        //cpsr
        add r0, r0, #4
        mrs r1, cpsr
        str r1, [r0]

        //spsr
        add r0, r0, #4
        mrs r1, spsr
        str r1, [r0]

    push {r2, r3}
    bl increment_thread //r0 now has the address of our next thread
    pop  {r2, r3}

    restore_thread:
          // stack is: r0, ... r12, sp, lr, pc, spsr
          add r0, r0, #4 // r0 = &CURRENT_THREAD.r0
          ldr r1, [r0] // r1 = thread.r0
          push {r1}
  
          add r0, r0, #4
          ldr r1, [r0] //r1 = thread.r1
          push {r1}
  
          add r0, r0, #4
          ldr r1, [r0] //r1 = thread.r2
          push {r1}
  
          add r0, r0, #4
          ldr r1, [r0] //r1 = thread.r3
          push {r1}
  
          add r0, r0, #4
          ldr r1, [r0] //r1 = thread.r4
          push {r1}
  
          add r0, r0, #4
          ldr r1, [r0] //r1 = thread.r5
          push {r1}
  
          add r0, r0, #4
          ldr r1, [r0] //r1 = thread.r6
          push {r1}
  
          add r0, r0, #4
          ldr r1, [r0] //r1 = thread.r7
          push {r1}
  
          add r0, r0, #4
          ldr r1, [r0] //r1 = thread.r8
          push {r1}
  
          add r0, r0, #4
          ldr r1, [r0] //r1 = thread.r9
          push {r1}
  
          add r0, r0, #4
          ldr r1, [r0] //r1 = thread.r10
          push {r1}
  
          add r0, r0, #4
          ldr r1, [r0] //r1 = thread.r11
          push {r1}
  
          add r0, r0, #4
          ldr r1, [r0] //r1 = thread.r12
          push {r1}
  
          add r0, r0, #4
          ldr r1, [r0] //r1 = thread.sp
          push {r1}
  
          add r0, r0, #4
          ldr r1, [r0] //r1 = thread.lr
          push {r1}
  
          add r0, r0, #4
          ldr r1, [r0] //r1 = thread.pc
          push {r1}

          add r0, r0, #8 //skip cpsr - agreed 
          ldr r1, [r0] //r1 = thread.spsr
          push {r1}

          // Our stack now looks like spsr, pc, lr, sp, r12, ... r0 (in order of popping)

          pop {r1} //this was the spsr - SPSR 

          pop {lr} //this was the pc   - PC (from thread)
          pop {r2} //this was the lr   - LR (from thread)
          pop {r3} //this was the sp   - SP (from thread)

          # push {r0-r12, lr}
          # mov  r0, lr
          # bl   check_registers // Used to check important registers
          # pop  {r0-r12, lr}

          // switch to SVC
          mrs r0, cpsr
          bic r0, r0, #0x1F
          orr r0, r0, #0x13
          msr cpsr_c, r0

              msr spsr, r1 // restore spsr
              mov lr, r2   // restore lr to be old lr
              mov sp, r3   // restore sp

          // switch to IRQ
          mrs r0, cpsr
          bic r0, r0, #0x1F
          orr r0, r0, #0x12
          msr cpsr_c, r0

          // our stack now just has the normal registers in it; restore them
          pop {r12}
          pop {r11}
          pop {r10}
          pop {r9}
          pop {r8}
          pop {r7}
          pop {r6}
          pop {r5}
          pop {r4}
          pop {r3}
          pop {r2}
          pop {r1}
          pop {r0}

    //enable interrupts
    push {r0}
    mrs r0,cpsr
    bic r0,r0,#0x80 // I=0 enables interrupts
    msr cpsr_c,r0
    pop {r0}

    mov  sp, #0x8000
    movs pc, lr

reset_asm:
  sub   lr, lr, #4
  push  {lr}
  push  {r0-r12}
  mov   r0, lr              @ Pass old pc
  bl    reset_vector        @ C function
  pop   {r0-r12}
  ldm   sp!, {pc}^

undefined_instruction_asm:
  sub   lr, lr, #4
  push  {lr}
  push  {r0-r12}
  mov   r0, lr                          @ Pass old pc
  bl    undefined_instruction_vector    @ C function
  pop   {r0-r12}
  ldm   sp!, {pc}^

software_interrupt_asm:
  sub   lr, lr, #4
  push  {lr}
  push  {r0-r12}
  mov   r0, lr                       @ Pass old pc
  bl    software_interrupt_vector    @ C function
  pop   {r0-r12}
  ldm   sp!, {pc}^

prefetch_abort_asm:
  sub   lr, lr, #4
  push  {lr}
  push  {r0-r12}
  mov   r0, lr                   @ Pass old pc
  bl    prefetch_abort_vector    @ C function
  pop   {r0-r12}
  ldm   sp!, {pc}^

data_abort_asm:
  sub   lr, lr, #4
  push  {lr}
  push  {r0-r12}
  mov   r0, lr                 @ Pass old pc
  bl    data_abort_vector      @ C function
  pop   {r0-r12}
  ldm   sp!, {pc}^
