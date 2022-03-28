#include "../h/scheduler.h"

void scheduler() {
    pcb_PTR HP_pcb = headProcQ(&(ready_hq->p_list));            /* Ready High-Priority pcb list  */
    pcb_PTR LP_pcb = headProcQ(&(ready_lq->p_list));            /* Ready Low-Priority pcb list */
    
    if (HP_pcb != NULL) {                                       /* Se ci sono dei pcb_h Ready */
        /* Aggiornamento del processo attuale */
        current_p = removeProcQ(&(ready_hq->p_list));
        LDST(&(current_p->p_s));
    } else if (HP_pcb == NULL && LP_pcb != NULL){               /* Se vi sono solo pcb_l */
        current_p = removeProcQ(&(ready_lq->p_list));
        setTIMER(TIMESLICE);
        LDST(&(current_p->p_s));
    } else if (HP_pcb == NULL && LP_pcb == NULL) {              /* Se non vi sono precessi ad {alta & bassa} priorità */
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