#include "../h/sysSupport.h"

extern int swap_pool_holding[UPROCMAX],
            swap_pool_semaphore,
            printer_sem[UPROCMAX],
            tread_sem[UPROCMAX],
            twrite_sem[UPROCMAX]; 


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
            write_to_printer(exception_state, curr_support->sup_asid);
            }
            break;
        case WRITETERMINAL: {
            write_to_terminal(exception_state, curr_support->sup_asid);
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
void write_to_printer (state_t *exception_state, int asid) {
    // Stringa da scrivere
    char *s = exception_state->reg_a1; 
    // Lunghezza della stringa da scrivere
    int len = exception_state->reg_a2; 
    
	// Ricavo l'indirizzo del device register associato alla stampante associata all'asid passato come parametro
	memaddr dev_reg_addr = (memaddr) (DEVREGSTRT_ADDR + ((PRNTINT - 3) * 0x80) + (asid * 0x10));    /* Indirizzo del device register della stampante */ 
    devreg_t *dev_reg = (devreg_t *) dev_reg_addr;

    // Errore, lunghezza non valida / indirizzo non valido
    if (len < 0 || len > MAXSTRLENG || s < KUSEG) terminate(asid);

    SYSCALL(PASSEREN, &printer_sem[asid], 0, 0); 
    for (int i = 0; i < len; i++){
        dev_reg->dtp.data0 = s[i]; 
        int status = SYSCALL(DOIO, &(dev_reg->dtp.command), TRANSMITCHAR, 0); 
        if (status != READY){
            SYSCALL(VERHOGEN, &printer_sem[asid], 0, 0); 
            exception_state->reg_v0 = -status; 
            return; 
        }
    }

    SYSCALL(VERHOGEN, &printer_sem[asid], 0, 0); 
    exception_state->reg_v0 = len;    
}

/* NSYS4 */
void write_to_terminal (state_t *exception_state, int asid) {
    // Stringa da scrivere
    char *s = exception_state->reg_a1; 
    // Lunghezza della stringa da scrivere
    int len = exception_state->reg_a2; 
    
	// Ricavo l'indirizzo del device register associato alla terminale dell'asid passato come parametro
	memaddr dev_reg_addr = (memaddr) (DEVREGSTRT_ADDR + ((TERMINT - 3) * 0x80) + (asid * 0x10));    /* Indirizzo del device register del terminale */ 
    devreg_t *dev_reg = (devreg_t *) dev_reg_addr;

    // Errore, lunghezza non valida / indirizzo non valido
    if (len < 0 || len > MAXSTRLENG || s < KUSEG) terminate(asid);

    SYSCALL(PASSEREN, &twrite_sem[asid], 0, 0); 
    for (int i = 0; i < len; i++){
        int status = SYSCALL(DOIO, &(dev_reg->term.transm_command), TRANSMITCHAR || (s[i] << 8), 0); 
        if (status & TERMSTATMASK != RECVD){
            SYSCALL(VERHOGEN, &twrite_sem[asid], 0, 0); 
            exception_state->reg_v0 = -status; 
            return; 
        }
    }

    SYSCALL(VERHOGEN, &twrite_sem[asid], 0, 0); 
    exception_state->reg_v0 = len;   
}

/* NSYS5 */
void read_from_terminal () {

}
