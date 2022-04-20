#ifndef EXCEPTIONS
#define EXCEPTIONS

#include "types.h"
#include "pandos_const.h"
#include "pandos_types.h"
#include "pcb.h"
#include "initial.h"

#define INTERVAL_INDEX 0


/* Gestore delle eccezioni */
void exception_handler(); 

/* Gestore delle syscall */
void syscall_handler(); 

/* Program Trap handler & TLB Exception handler */
void pass_up_or_die(int index_value, state_t *exception_state); 


/* 
    Syscalls 
*/

/* NSYS1 */
void create_process(state_t *a1_state, int a2_p_prio, support_t *a3_p_support_struct); 

/* NSYS2 */
void terminate_process(int a1_pid); 
/* Funzione ausiliaria ricorsiva che termina l'intera discendenza del processo old_proc (incluso old_proc) */
void terminate_all(pcb_PTR old_proc);

/* NSYS3 & NSYS4*/
void sem_operation(int *a1_semaddr, int *block_flag, int p_flag);

/* NSYS5 */
void do_io(int *a1_cmdAddr, int a2_cmdValue, int *block_flag);

/* NSYS6 */
void get_cpu_time();

/* NSYS7 */
void wait_for_clock(int *block_flag);

/* NSYS8 */
void get_support_data();

/* NSYS9 */
void get_processor_id(int a1_parent);

/* NSYS10 */
void yield(int *block_flag, int *low_priority);

/* Utility function */
void ready_by_priority(pcb_PTR to_insert); 

#endif