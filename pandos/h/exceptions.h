#ifndef EXCEPTIONS
#define EXCEPTIONS

#include "./types.h"
#include "./pandos_const.h"
#include "./pandos_types.h"
#include "./pcb.h"


/* Stato del processore al momento dell'eccezione */
state_t *exception_state; 

/* Processo responsabile dell'eccezione */
extern pcb_PTR current_p; 

/* Ready queues */
extern pcb_PTR ready_hq; 
extern pcb_PTR ready_lq; 

/* Gestore delle eccezioni */
void exception_handler(); 

/* Gestore delle syscall */
void syscall_handler(); 

/* Gestore delle eccezioni TLB */
void tlb_handler(); 

/* Gestore delle trap */
void trap_handler(); 


/* 
    Syscalls 
*/

/* NSYS1 */
void create_process(state_t *a1, int p_prio, support_t *p_support_struct); 

/* NSYS2 */


#endif