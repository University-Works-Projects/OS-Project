#include "../h/exceptions.h"


void exception_handler(){
    /* Recupero dello stato al momento dell'eccezione del processore */
    exception_state = BIOSDATAPAGE; 

    /* And bitwise per estrarre il Cause.ExcCode */
    int cause = exception_state->cause & GETEXECCODE; 

    /* Shift per migliore manipolazione */
    cause = cause >> 2; 

    /* Switch per la gestione dell'eccezione */
    switch(cause){
        case IOINTERRUPTS:                                  /* Si è verificato un interrupt */
            interrupt_handler(); 
            break; 
        case 1:                                             /* Si è verificata una eccezione TLB*/ 
        case TLBINVLDL:
        case TLBINVLDS:
            tlb_handler(); 
            break; 
        case SYSEXCEPTION:                                  /* E' stata chiamata una system call */
            if (exception_state->status & USERPON)          /* La chiamata è avvenuta in kernel mode */
                syscall_handler(); 
            else                                            /* La chiamata non è avvenuta in kernel mode */
                trap_handler(); 
            break;
        default:                                            /* E' scattata una trap */
            trap_handler(); 
            break; 
    }

}

void syscall_handler(){
    /* Intero che rappresenta il tipo di system call */
    int syscode = exception_state->reg_a0;
    
    /* Switch per la gestione della syscall */
    switch(syscode){
        case CREATEPROCESS:
            create_process(); 
            break; 
        case TERMPROCESS:
            break; 
        case PASSEREN:
            break; 
        case VERHOGEN:
            break; 
        case DOIO:
            break; 
        case GETTIME:
            break; 
        case CLOCKWAIT:
            break; 
        case GETSUPPORTPTR:
            break; 
        case GETPROCESSID:
            break; 
        case YIELD:
            break; 
    }

}

void create_process(){

}

