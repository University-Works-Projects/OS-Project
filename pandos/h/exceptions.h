#ifndef EXCEPTIONS
#define EXCEPTIONS

#include "types.h"
#include "pandos_const.h"
#include "pandos_types.h"
#include "pcb.h"
#include "initial.h"

#define DEVICE_EXCEPTIONS 48

extern cpu_t start_usage_cpu; 

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
void create_process(state_t *a1_state, int a2_p_prio, support_t *a3_p_support_struct); 

/* NSYS2 */
void terminate_process(int a2_pid); 
/* Funzione ausiliaria ricorsiva che termina l'intera discendenza del processo old_proc (incluso old_proc) */
void terminate_all(pcb_PTR old_proc){

/* NSYS3 */
void passeren (int *a1_semaddr, int *block_flag);

/* NSYS4 */
void verhogen (int *a1_semaddr);

/* NSYS5 */
int do_io(int *a1_cmdAddr, int a2_cmdValue, int *block_flag);

/* NSYS6 */
int get_cpu_time();

/* NSYS7 */
int wait_for_clock(int *block_flag);

/* NSYS8 */
support_t* get_support_data();

/* NSYS9 */
int get_processor_id(int a1_parent);

/* NSYS10 */
int yield();

#endif