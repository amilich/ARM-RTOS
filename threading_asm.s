
# .globl yield
# yield:
#     # save_current_thread:
#         //remember r1 so you can use it for r0
#         push {r1}
#         mov r1, r0 //store r0 so it can be restored

#         push {r1-r12, lr}
#         bl get_current_thread  //r0 now has the address of CURRENT_THREAD
#         pop {r1-r12, lr}

#         add r0, r0, #4   // r0 = &CURRENT_THREAD.r0
#         str r1, [r0]     // save what the r0 was

#         pop {r1}         // restore r1
#         add r0, r0, #4   // r0 = &CURRENT_THREAD.r1
#         str r1, [r0]     // save r1

#         //r2
#         add r0, r0, #4
#         str r2, [r0]

#         //r3
#         add r0, r0, #4
#         str r3, [r0]

#         //r4
#         add r0, r0, #4
#         str r4, [r0]

#         //r5
#         add r0, r0, #4
#         str r5, [r0]

#         //r6
#         add r0, r0, #4
#         str r6, [r0]

#         //r7
#         add r0, r0, #4
#         str r7, [r0]

#         //r8
#         add r0, r0, #4
#         str r8, [r0]

#         //r9
#         add r0, r0, #4
#         str r9, [r0]

#         //r10
#         add r0, r0, #4
#         str r10, [r0]

#         //r11
#         add r0, r0, #4
#         str r11, [r0]

#         //r12
#         add r0, r0, #4
#         str r12, [r0]

#         //sp
#         add r0, r0, #4
#         str sp, [r0]

#         //lr
#         add r0, r0, #4
#         str lr, [r0]

#         //pc (Though we're actually saving lr)
#         add r0, r0, #4
#         str lr, [r0]

#         //cpsr
#         add r0, r0, #4
#         mrs r1, cpsr
#         str r1, [r0]

#         //spsr
#         add r0, r0, #4
#         mrs r1, spsr
#         str r1, [r0]

#     push {r1-r12, lr}
#     bl increment_thread_cooperative //r0 now has the address of our next thread
#     pop  {r1-r12, lr}

#     restore_thread:
#           // stack is going to be: r1, ... r12, sp, lr, pc
#           add r0, r0, #4 // r0 = &CURRENT_THREAD.r0
#           ldr r1, [r0] // r1 = thread.r0
#           push {r1}

#           add r0, r0, #4
#           ldr r1, [r0] //r1 = thread.r1
#           push {r1}

#           add r0, r0, #4
#           ldr r1, [r0] //r1 = thread.r2
#           push {r1}

#           add r0, r0, #4
#           ldr r1, [r0] //r1 = thread.r3
#           push {r1}

#           add r0, r0, #4
#           ldr r1, [r0] //r1 = thread.r4
#           push {r1}

#           add r0, r0, #4
#           ldr r1, [r0] //r1 = thread.r5
#           push {r1}

#           add r0, r0, #4
#           ldr r1, [r0] //r1 = thread.r6
#           push {r1}

#           add r0, r0, #4
#           ldr r1, [r0] //r1 = thread.r7
#           push {r1}

#           add r0, r0, #4
#           ldr r1, [r0] //r1 = thread.r8
#           push {r1}

#           add r0, r0, #4
#           ldr r1, [r0] //r1 = thread.r9
#           push {r1}

#           add r0, r0, #4
#           ldr r1, [r0] //r1 = thread.r10
#           push {r1}

#           add r0, r0, #4
#           ldr r1, [r0] //r1 = thread.r11
#           push {r1}

#           add r0, r0, #4
#           ldr r1, [r0] //r1 = thread.r12
#           push {r1}

#           add r0, r0, #4
#           ldr r1, [r0] //r1 = thread.sp
#           push {r1}

#           add r0, r0, #4
#           ldr r1, [r0] //r1 = thread.lr
#           push {r1}

#           add r0, r0, #4
#           ldr r1, [r0] //r1 = thread.pc
#           push {r1}

#           add r0, r0, #8 //skip cpsr
#           ldr r1, [r0] //r1 = thread.spsr
#           //push {r1} //temporarily ignoring spsr

#           // Our stack now looks like pc, lr, sp, r12, ... r0 (in order of popping)

#           //pop {r0} //this was the spsr - SPSR
#           //msr spsr, r0 // restore spsr

#           pop {r0} //this was the pc   - PC (from thread)
#           pop {lr} //this was the lr   - LR (from thread)
#           pop {r0} //this was the sp   - SP (from thread)

#           // our stack now just has the normal registers in it; restore them
#           pop {r12}
#           pop {r11}
#           pop {r10}
#           pop {r9}
#           pop {r8}
#           pop {r7}
#           pop {r6}
#           pop {r5}
#           pop {r4}
#           pop {r3}
#           pop {r2}
#           pop {r1}
#           pop {r0}

#           //mov sp, r0   // restore sp

#     bx lr

# .globl asm_push
# asm_push: 
#   bl system_disable_interrupts 
#   push {r0-r12, lr}^ 
#   bx lr 

# .globl asm_pop
# asm_pop: 
#   bl system_enable_interrupts 
#   pop {r0-r12, lr}^ 
#   bx lr 

