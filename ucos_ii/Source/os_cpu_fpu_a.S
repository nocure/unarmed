@
@********************************************************************************************************
@                                               uC/OS-II
@                                         The Real-Time Kernel
@
@
@                             (c) Copyright 1992-2007, Micrium, Weston, FL
@                                          All Rights Reserved
@
@                                           Generic ARM Port
@                                              VFP SUPPORT
@
@ File      : OS_CPU_FPU_A.ASM
@ Version   : V1.84
@ By        : Eric Shufro
@
@ For       : ARM7 or ARM9
@ Mode      : ARM or Thumb
@ Toolchain : GNU GCC
@********************************************************************************************************
@

@********************************************************************************************************
@                                           PUBLIC FUNCTIONS
@********************************************************************************************************

    .global  OS_CPU_FP_Restore
    .global  OS_CPU_FP_Save
#if 0
@********************************************************************************************************
@                                      CODE GENERATION DIRECTIVES
@********************************************************************************************************

    .code 32

@*********************************************************************************************************
@                                           RESTORE VFP REGISTERS
@                                       void OS_CPU_FP_Restore(void *pblk)
@
@ Description : This function is called to restore the contents of the VFP registers during a context
@               switch.  It is assumed that a pointer to a storage area for the VFP registers is placed
@               in the task's TCB (i.e. .OSTCBExtPtr).
@
@ Arguments   : pblk    is passed to this function in R0 when called.
@
@ Notes       : Floating point math should NEVER be performed within an ISR as this will corrupt
@               the state of the VFP registers for the last VFP task that ran. Instead, all floating
@               point math should be performed within floating point enabled tasks ONLY.
@*********************************************************************************************************

OS_CPU_FP_Restore:
        FLDMIAS R0!, {S0-S31}           @ Restore the VFP registers from pblk
        BX      LR                      @ Return to calling function

@*********************************************************************************************************
@                                           SAVE VFP REGISTERS
@                                        void OS_CPU_FP_Save(void *pblk)
@
@ Description : This function is called to save the contents of the VFP registers during a context
@               switch.  It is assumed that a pointer to a storage area for the VFP registers is placed
@               in the task's TCB (i.e. .OSTCBExtPtr).
@
@ Arguments   : pblk    is passed to this function in R0 when called.
@
@ Notes       : Floating point math should NEVER be performed within an ISR as this will corrupt
@               the state of the VFP registers for the last VFP task that ran. Instead, all floating
@               point math should be performed within floating point enabled tasks ONLY.
@*********************************************************************************************************

OS_CPU_FP_Save:
        FSTMIAS R0!, {S0-S31}           @ Save the VFP registers in pblk
        BX      LR                      @ Return to calling function


        .ltorg
#endif
