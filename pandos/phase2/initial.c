#include "../h/initial.h"

extern void test();
extern void uTLB_RefillHandler();
extern void exception_handler();
extern void scheduler();

struct list_head ready_hq; 
struct list_head ready_lq; 

int main () {


    /* Inizializzazione variabili globali */
    p_count = 0, soft_counter = 0;
    mkEmptyProcQ(&(ready_hq));                                    /* Inizializzazione delle liste e dei campi*/
    mkEmptyProcQ(&(ready_lq));                                  /* Inizializzazione delle liste e dei campi*/
    current_p = NULL;
    
    for (int i=0; i<DEVICE_INITIAL; i++)            /* Inizializzazione semafori associati ai device */
        sem[i] = 0;

    /* Inizializzazione passupvector */
    passupvector = (passupvector_t*) PASSUPVECTOR;
    passupvector->tlb_refill_handler = (memaddr) uTLB_RefillHandler;
    passupvector->exception_handler = (memaddr) exception_handler;
    passupvector->tlb_refill_stackPtr = KERNELSTACK; 
    passupvector->exception_stackPtr = KERNELSTACK; 

    /* Inizializzazione delle strutture dati di fase 1 */
    initPcbs();
    initASL();

    /* Interval timer di 100 ms (in microseconds) */
	LDIT(100000);

    /* Dichiarazione del processo da iniziare e inizializzazione */
    pcb_PTR new_p = allocPcb(); 
    STST(&(new_p->p_s)); 
    insertProcQ(&(ready_lq), new_p); 
    /* processor Local Timer abilitato, Kernel-mode on, Interrupts Abilitati */
    (new_p->p_s).status = TEBITON | IEPON | IMON;
    /* Inizializzazione sp */
    RAMTOP((new_p->p_s).reg_sp);

    (new_p->p_s).pc_epc = (memaddr) test; 

    /* Nuovo processo "iniziato" */
    p_count++;

    scheduler(); 
    return 0;
}