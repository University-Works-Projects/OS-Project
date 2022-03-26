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
            state_t *a1 = (state_t *) exception_state->reg_a1; 
            int p_prio = (int) exception_state->reg_a2; 
            support_t *p_support_struct = (support_t *) exception_state->reg_a3; 
            create_process(a1,p_prio,p_support_struct); 
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

void create_process(state_t *a1, int p_prio, support_t *p_support_struct){
    /* Tentativo di allocazione di un nuovo processo */
    pcb_PTR new_proc = allocPcb(); 

    if (new_proc != NULL){                           /* Allocazione avvenuta con successo */
        /* Il nuovo processo è figlio del processo chiamante */
        insertChild(current_p,new_proc); 

        /* Inizializzazione campi del nuovo processo a partire da a1,a2,a3 */
        new_proc->p_s = *a1; 
        new_proc->p_prio = p_prio; 
        new_proc->p_supportStruct = p_support_struct; 

        /* PID è implementato come l'indirizzo del pcb_t */
        new_proc->p_pid = new_proc; 


        /* Il nuovo processo è pronto per essere messo nella ready_q */
        switch(new_proc->p_prio){
            case 0:                                 /* E' un processo a bassa priorità */
                insertProcQ(&(ready_lq->p_list),new_proc); 
                break;
            case 1:                                 /* E' un processo ad alta priorità */
                insertProcQ(&(ready_hq->p_list),new_proc); 
                break; 
        }

        /* Operazione completata, ritorno con successo */
        current_p->p_s.reg_v0 = new_proc->p_pid; 
    }else                                           /* Allocazione fallita, risorse non disponibili */
        current_p->p_s.reg_v0 = NOPROC; 

    //TODO: Leggere e implementare sezione 3.5.12 manuale pandosplus per il "return from a syscall"
}

