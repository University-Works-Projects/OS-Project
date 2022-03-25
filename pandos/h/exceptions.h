#ifndef EXCEPTIONS
#define EXCEPTIONS

#include "./types.h"
#include "./pandos_const.h"
#include "./pandos_types.h"

/* Stato del processore al momento dell'eccezione */
state_t *exception_state; 

/* Gestore delle eccezioni */
void exception_handler(); 

/* Gestore delle syscall */
void syscall_handler(); 

/* Gestore delle eccezioni TLB */
void tlb_handler(); 

/* Gestore delle trap */
void trap_handler(); 

#endif