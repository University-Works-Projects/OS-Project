#include "../h/scheduler.h"

void scheduler() {
    pcb_PTR HP_pcb = headProcQ(&(ready_hq));            /* Ready High-Priority pcb list  */
    pcb_PTR LP_pcb = headProcQ(&(ready_lq));            /* Ready Low-Priority pcb list */

    cpu_t now; 
    STCK(now); 
    /* Aggiornamento del tempo di uso della CPU: CURRENT_TOD - START_USAGE_TOD (sezione 3.8 del manuale di pandosplus) */
    if(current_p != NULL) current_p->p_time += now - start_usage_cpu; 

    if (HP_pcb != NULL) {                                       /* Se ci sono dei pcb_h Ready */
        /* Aggiornamento del processo attuale */
        current_p = removeProcQ(&(ready_hq));
        LDST(&(current_p->p_s));
        /* Tempo di inizio di uso della CPU */
        STCK(start_usage_cpu); 
    } else if (HP_pcb == NULL && LP_pcb != NULL){               /* Se vi sono solo pcb_l */
        current_p = removeProcQ(&(ready_lq));
        setTIMER(TIMESLICE);
        LDST(&(current_p->p_s));
        /* Tempo di inizio di uso della CPU */
        STCK(start_usage_cpu); 
    } else if (HP_pcb == NULL && LP_pcb == NULL) {              /* Se non vi sono precessi ad {alta & bassa} priorità */
        if (p_count == 0)
            HALT();
        else if (p_count > 0) {
            if (soft_counter > 0) {
                setSTATUS(IECON);                               /* Abilitazione degli interrupts e (automatica) disabilitazione del PLT */
                WAIT();
            } else if (soft_counter == 0)                       /* Deadlock */
                PANIC();
        }
    }
}