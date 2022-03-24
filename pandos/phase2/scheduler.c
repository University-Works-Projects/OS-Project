#include "../h/scheduler.h"

void scheduler() {
    pcb_PTR proc_h = headProcQ(&(ready_hq->p_list));
    pcb_PTR proc_l = headProcQ(&(ready_lq->p_list));
    
    if (proc_h != NULL) {                                       /* Se ci sono dei processi Ready */
        /* Aggiornamento del processo attuale */
        current_p = removeProcQ(&(ready_hq->p_list));
        LDST(&(current_p->p_s));
    } else if (proc_h == NULL && proc_l != NULL){
        current_p = removeProcQ(&(ready_lq->p_list));
        // setTIMER(5 ms);
        LDST(&(current_p->p_s));
    } else if (proc_h == NULL && proc_l == NULL) {              /* Se non vi sono precessi ad {alta & bassa} priorità */
        if (p_count == 0)
            HALT();
        else if (p_count > 0) {
            if (soft_counter > 0) {
                setSTATUS(IECON | IMON);                        /* Abilitazione degli interrupts e (automatica) disabilitazione del PLT */
                WAIT();
            } else if (soft_counter == 0)
                PANIC();
        }
    }
}