#include "../h/scheduler.h"

// Coda dei processi ad alta priotita'
extern struct list_head ready_hq;               
// Coda dei processi a bassa priotita'
extern struct list_head ready_lq;               
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
        // Aggiornamento del tempo di uso della CPU: CURRENT_TOD - START_USAGE_TOD (3.8 pandosplus)
        current_p->p_time += now - start_usage_cpu;             

    if (HP_pcb != NULL) {                                       
        // Se ci sono dei processi ad alta priorita' in stato ready
        current_p = removeProcQ(&(ready_hq));                       
        LDST(&(current_p->p_s));
        // Tempo di inizio di uso della CPU
        STCK(start_usage_cpu);                                      
    } else if (HP_pcb == NULL && LP_pcb != NULL) {               
        // Se vi sono solo processi a bassa priorita' 
        current_p = removeProcQ(&(ready_lq));
        setTIMER(TIMESLICE);
        // Tempo di inizio di uso della CPU
        STCK(start_usage_cpu); 
        LDST(&(current_p->p_s));
    } else if (HP_pcb == NULL && LP_pcb == NULL) {              
        // Se non vi sono precessi ad {alta & bassa} priorità
        if (p_count == 0)
            HALT();
        else if (p_count > 0) {
            if (soft_counter > 0) {
                // Abilitazione degli interrupts e (automatica) disabilitazione del PLT
                setTIMER((unsigned int) NULL);
                setSTATUS(IMON | IECON);
                WAIT();
            } else if (soft_counter == 0)                           
                // Deadlock
                PANIC();
        }
    }
}