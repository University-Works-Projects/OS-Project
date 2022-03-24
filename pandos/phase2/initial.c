#include "../h/initial.h"

int main () {

    /* Inizializzazione variabili globali */
    p_count = 0, soft_counter = 0;
    mkEmptyProcQ(&ready_q->p_list);         /* puntatore alla tail della coda dei pcb in stato ready */
    current_p = NULL;
    
    for (int i=0; i<N_DEVICE; i++)          /* Inizializzazione semafori associati ai device */
        sem[i] = 0;

    /* Inizializzazione passupvector */
    passupvector = PASSUPVECTOR; 
    passupvector->tlb_refill_handler = (memaddr) uTLB_RefillHandler;
    passupvector->exception_handler = (memaddr) exception_handler;
    passupvector->tlb_refill_stackPtr = KERNELSTACK; 
    passupvector->exception_stackPtr = KERNELSTACK; 

    /* Inizializzazione delle strutture dati di fase 1 */
    intPcbs();
    intSemd();

    /* Interval timer di 100 ms (in microseconds) */
	LDIT(100000);

    /* Nuovo processo */
    pcb_PTR new_p = allocPcb(); 
    
    insertProcQ(ready_q, new_p); 
    /* processor Local Timer abilitato, Kernel-mode on, Interrupts Abilitati */
    (new_p->p_s).status = TEBITON | IEPON | IMON;
    RAMTOP((new_p->p_s).reg_sp);

    (new_p->p_s).pc_epc = (memaddr) test; 


    scheduler(); 
    return 0;
}