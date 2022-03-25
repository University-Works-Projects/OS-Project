#include "../h/exceptions.h"


void exception_handler(){
    /* Recupero dello stato al momento dell'eccezione del processore */
    exception_state = BIOSDATAPAGE; 

    /* And bitwise per estrarre il Cause.ExcCode */
    int cause = exception_state->cause & GETEXECCODE; 

    /* Switch per la gestione dell'eccezione */
    switch(cause){
        case IOINTERRUPTS:                  /* Si è verificato un interrupt */
            interrupt_handler(); 
            break; 
        case 1:                             /* Si è verificata una eccezione TLB*/ 
        case TLBINVLDL:
        case TLBINVLDS:
            tlb_handler(); 
            break; 
        case SYSEXCEPTION:                  /* E' stata chiamata una system call */
            syscall_handler(); 
            break;
        default:                            /* E' scattata una trap */
            trap_handler(); 
            break; 
    }

}