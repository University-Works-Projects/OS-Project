#include "../h/initial.h"

/* Declaring the functions that will be used in the main.c file. */
extern void test();
extern void uTLB_RefillHandler();
extern void exception_handler();
extern void scheduler();

struct list_head ready_hq; 
struct list_head ready_lq;

/* Array dei semafori dei dispositivi */
int sem[DEVICE_INITIAL];
/* Intero che rappresenta rispettivamente il numero di processi "vivi" e il numero di processi bloccati per I/O */
int p_count, soft_counter;
/* Coda dei processi in stato ready ad alta (hq) e bassa (lq) priorità */

pcb_PTR current_p;

passupvector_t *passupvector;

int main () {

    /* Inizializzazione variabili globali */
    p_count = 0, soft_counter = 0;
    mkEmptyProcQ(&(ready_hq));
    mkEmptyProcQ(&(ready_lq));
    current_p = NULL;
    
    /* Inizializzazione semafori associati ai device */
    for (int i = 0; i < DEVICE_INITIAL; i++)
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
    insertProcQ(&(ready_lq), new_p);
    
    /* processor Local Timer abilitato, Kernel-mode on, Interrupts Abilitati */
    (new_p->p_s).status = TEBITON | IEPON | IMON;
    
    /* Inizializzazione sp */
    RAMTOP((new_p->p_s).reg_sp);

    (new_p->p_s).pc_epc = (memaddr) test; 
    (new_p->p_s).reg_t9 = (memaddr) test; 

    /* Nuovo processo "iniziato" */
    p_count++;

    scheduler(); 
    return 0;
}