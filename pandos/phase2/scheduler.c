#include "../h/scheduler.h"

extern struct list_head ready_hq;               /* Ready High-Priority pcb list */
extern struct list_head ready_lq;               /* Ready Low-Priority pcb list */
extern pcb_PTR current_p; 
extern int p_count; 
extern int soft_counter; 

cpu_t start_usage_cpu;

void scheduler() {
    pcb_PTR HP_pcb = headProcQ(&(ready_hq));
    pcb_PTR LP_pcb = headProcQ(&(ready_lq));
    cpu_t now; 
    STCK(now); 
    
    if (current_p != NULL)
        current_p->p_time += now - start_usage_cpu;             /* Aggiornamento del tempo di uso della CPU: CURRENT_TOD - START_USAGE_TOD (3.8 pandosplus) */ 

    if (HP_pcb != NULL) {                                       /* Se ci sono dei pcb_h Ready */
        current_p = removeProcQ(&(ready_hq));                       /* Aggiornamento del processo attuale */
        LDST(&(current_p->p_s));
        STCK(start_usage_cpu);                                      /* Tempo di inizio di uso della CPU */
    } else if (HP_pcb == NULL && LP_pcb != NULL) {               /* Se vi sono solo pcb_l */
        current_p = removeProcQ(&(ready_lq));
        setTIMER(TIMESLICE);
        LDST(&(current_p->p_s));
        STCK(start_usage_cpu); 
    } else if (HP_pcb == NULL && LP_pcb == NULL) {              /* Se non vi sono precessi ad {alta & bassa} priorità */
        if (p_count == 0)
            HALT();
        else if (p_count > 0) {
            if (soft_counter > 0) {
                setSTATUS(IECON | IMON);                            /* Abilitazione degli interrupts e (automatica) disabilitazione del PLT */
                WAIT();
            } else if (soft_counter == 0)                           /* Deadlock */
                PANIC();
        }
    }
}