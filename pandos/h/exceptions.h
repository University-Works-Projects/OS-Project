#ifndef EXCEPTIONS
#define EXCEPTIONS

#include "types.h"
#include "pandos_const.h"
#include "pandos_types.h"
#include "pcb.h"
#include "initial.h"

#define DEVICE_EXCEPTIONS 48

/* Stato del processore al momento dell'eccezione */
state_t *exception_state; 

/* Processo responsabile dell'eccezione */
extern pcb_PTR current_p; 

/* Ready queues */
extern pcb_PTR ready_hq; 
extern pcb_PTR ready_lq; 

/* Processi vivi */
extern int p_count; 
/* Processi bloccati, che stanno aspettando una operazione di I/O */
extern int soft_counter; 

/* Semafori associati ai dispositivi */
extern int sem[DEVICE_EXCEPTIONS]; 

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
void terminate_process(int pid); 
/* Funzione ausiliaria ricorsiva che termina l'intera discendenza del processo old_proc (incluso old_proc) */
void terminate_all(pcb_PTR old_proc){

/* NSYS3 */

#endif