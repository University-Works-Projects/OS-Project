#ifndef EXCEPTIONS
#define EXCEPTIONS

#include "types.h"
#include "pandos_const.h"
#include "pandos_types.h"
#include "pcb.h"
#include "initial.h"

#define INTERVAL_INDEX 0


/**
 * Il gestore delle eccezioni e' chiamato quando occore un'eccezione. Determina la causa dell'eccezione 
 * e chiama il gestore appropriato.
 */
void exception_handler(); 

/**
 * Il gestore delle system call chiama la system call appropriata e chiama lo scheduler se necessario. 
 */
void syscall_handler(); 

// NSYS
/**
 * NSYS1 - crea un nuovo processo, lo inizialiazza e lo inserisce nella ready queue
 * 
 * @param a1_state lo stato del processo da creare
 * @param a2_p_prio la priorita' del processo da creare
 * @param a3_p_support_struct il puntatore alla support_t struct del processo da creare
 */
void create_process(state_t *a1_state, int a2_p_prio, support_t *a3_p_support_struct); 

/**
 * NSYS2 - Rimuove il processo con pid a1_pid dalla lista dei figli del padre, e termina tutta la sua discendenza 
 * @param a2_pid Il pid del processo da terminare.
 */
void terminate_process(int a1_pid);
/**
 * Rimuove tutti i figli (ricorsivamente) del processo passato come parametro e li libera
 * @param old_proc Il pcb del processo da terminare
 */
void terminate_all(pcb_PTR old_proc);

/**
 * NSYS3 & NSYS4 - Effettua una P o V (binaria) in base al valore di p_flag 
 * @param a1_semaddr il semaforo su cui fare l'operazione 
 * @param block_flag flag che indica se il processo corrente e' da bloccare o no
 * @param p_flag flag che indica se si tratta di un P
 */
void sem_operation(int *a1_semaddr, int *block_flag, int p_flag);

/**
 * NSYS5 - Inizia una operazione di I/O sul device con indirizzo di registro a1_cmdAddr
 * 
 * @param a1_cmdAddr indirizzo del command field del device register da accedere
 * @param a2_cmdValue valore da scrivere nel cmdAddr 
 * @param block_flag flag che indica se il processo corrente e' da bloccare o no
 */
void do_io(int *a1_cmdAddr, int a2_cmdValue, int *block_flag);

/**
 * NSYS6 - Ritorna il tempo totale di utilizzo della CPU 
 */
void get_cpu_time();

/**
 * NSYS7 - Aspetta che l'interval timer generi un interrupt
 * 
 * @param block_flag flag che indica se il processo corrente e' da bloccare o no
 */
void wait_for_clock(int *block_flag);

/**
 * NSYS8 - Ritorna l'indirizzo della support structure del processo corrente 
 */
void get_support_data();

/**
 * NSYS9 - Se il pid del padre e' 0, ritorna il pcb del processo corrente, altrimenti ritorna il pcb del padre 
 * 
 * @param a1_parent 0 if you want the current process, 1 if you want the parent process
 */
void get_processor_id(int a1_parent);

/**
 * NSYS10 - E' utilizzata per reinserire il processo corrente nella ready queue e continuare con l'esecuzione di un altro processo 
 * 
 * @param block_flag flag che indica se il processo corrente e' da bloccare o no
 * @param low_priority flag che indica se lo scheduler dovra' scegliere un processo a bassa priorita' 
 */
void yield(int *block_flag, int *low_priority);



/**
 * Se il processo corrente ha un support structure, copia lo stato al momento dell'eccezione dentro di essa,
 * poi carica il context del general exception handler nella CPU
 * 
 * @param index_value indice dell'eccezione del vettore delle eccezioni 
 * @param exception_state stato al momento in cui si e' verificata l'eccezione 
 */
void pass_up_or_die(int index_value, state_t *exception_state); 

/**
 * Inserimento del pcb nella coda di ready appropriata in base alla sua prioritia'
 * 
 * @param to_insert pcb da inserire 
 */
void ready_by_priority(pcb_PTR to_insert);

/**
 * @brief Prende l'indirizzo della pagina che ha causato l'eccezione TLB Miss e la carica nel TLB
 */
void uTLB_RefillHandler(); 

#endif