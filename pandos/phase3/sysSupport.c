#include "../h/sysSupport.h"

extern int swap_pool_holding[UPROCMAX]; 
extern int swap_pool_semaphore = 1; 


void general_exception_handler() {
    support_t* curr_support = (support_t*) SYSCALL(GETSUPPORTPTR, 0, 0, 0);     /* SYS8 to get the supp struct's addrs of the curr proc */
    // Ricavo lo stato al momento dell'eccezione
    state_t *exception_state = &(curr_support->sup_exceptState[GENERALEXCEPT]); 

	// Estrazione del Cause.ExcCode
    int exe_code = exception_state->cause & GETEXECCODE;
    if (9 <= exe_code && exe_code >= 12)
        terminate(exception_state);
    // Intero che rappresenta la syscall chiamata
    int syscode = exception_state->reg_a0;

    // Intero che rappresenta lo status
    int status = exception_state->status;

    switch (syscode) {
        case GET_TOD: {
            get_tod(exception_state);
            }
            break;
        case TERMINATE: {
            terminate(curr_support->sup_asid);
            }
            break;
        case WRITEPRINTER: {
            write_to_printer(exception_state);
            }
            break;
        case WRITETERMINAL: {
            write_to_terminal();
            }
            break;
        case READTERMINAL: {
            read_from_terminal();
            }
            break;
        default: {
            }
            break;
    }
    // Any return status is placed in v0
    exception_state->pc_epc += WORDLEN;
    LDST(exception_state);
}

/* NSYS11 */
void get_tod (state_t *exception_state) {
    unsigned int tod; 
    STCK(tod); 
    exception_state->reg_v0 = tod; 
}

/* NSYS2 */
void terminate (int asid) {
   /**
    * If the process to be terminated is currently holding mutual exclusion on
    * a Support Level semaphore (e.g. Swap Pool semaphore), mutual exclusion must
    * first be released (NSYS4) before invoking the Nucleus terminate command (NSYS2).
    */
    if (swap_pool_holding[asid]){
        swap_pool_holding[asid] = 0; 
        SYSCALL(VERHOGEN, &swap_pool_semaphore, 0, 0); 
    }
    SYSCALL(TERMPROCESS, 0, 0, 0); 
}

/* NSYS3 */
void write_to_printer (state_t *exception_state) {
    // Intero che rappresenta il numero di caratteri trasmessi
    int transmitted = 0; 
    // Stringa da scrivere
    char *s = exception_state->reg_a1; 
    // Lunghezza della stringa da scrivere
    int len = exception_state->reg_a2; 


    exception_state->reg_v0 = transmitted;    
}

/* NSYS4 */
void write_to_terminal () {
    
}

/* NSYS5 */
void read_from_terminal () {
    
}
