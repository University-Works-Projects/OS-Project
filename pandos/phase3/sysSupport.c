#include "../h/sysSupport.h"

extern int swap_pool_holding[UPROCMAX],
            swap_pool_semaphore,
            printer_sem[UPROCMAX],
            tread_sem[UPROCMAX],
            twrite_sem[UPROCMAX]; 

extern int master_semaphore; 
extern swap_t swap_pool[POOLSIZE]; 

void general_exception_handler() {
    // Ottengo la struttura di supporto del processo corrente
    support_t* curr_support = (support_t*) SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    // Ricavo lo stato al momento dell'eccezione
    state_t *exception_state = &(curr_support->sup_exceptState[GENERALEXCEPT]); 

	// Estrazione del Cause.ExcCode
    int cause = (exception_state->cause & GETEXECCODE) >> 2;
    // L'eccezione deve essere trattata come una trap, ovvero come una SYS2
    if (cause != SYSEXCEPTION) terminate(curr_support->sup_asid - 1);
    // Intero che rappresenta la syscall chiamata
    int syscode = exception_state->reg_a0;

    switch (syscode) {
        case GET_TOD: 
            get_tod(exception_state);
            break;
        case TERMINATE: 
            terminate(curr_support->sup_asid - 1);
            break;
        case WRITEPRINTER: 
            write_to_printer(exception_state, curr_support->sup_asid - 1);
            break;
        case WRITETERMINAL: 
            write_to_terminal(exception_state, curr_support->sup_asid - 1);
            break;
        case READTERMINAL: 
            read_from_terminal(exception_state, curr_support->sup_asid - 1);
            break;
        default: 
            terminate(curr_support->sup_asid - 1);
            break;
    }
    exception_state->pc_epc += WORDLEN;
    exception_state->reg_t9 += WORDLEN;
    LDST(exception_state);
}

// SYS1
void get_tod (state_t *exception_state) {
    unsigned int tod; 
    STCK(tod); 
    exception_state->reg_v0 = tod; 
}

// SYS2
void terminate (int asid) {
    // I frame occupati dal processo che deve essere terminato, devono essere marcati liberi
    for (int i = 0; i < POOLSIZE; i++){
        if (swap_pool[i].sw_asid == asid + 1){
            swap_pool[i].sw_asid = NOPROC; 
        }
    }
    // La mutua esclusione sulla swap pool table deve essere rilasciata prima di terminare
    if (swap_pool_holding[asid]){
        swap_pool_holding[asid] = 0; 
        SYSCALL(VERHOGEN, (memaddr) &swap_pool_semaphore, 0, 0); 
    }
    // Sblocca test
    SYSCALL(VERHOGEN, (memaddr) &master_semaphore, 0, 0);
    // Termina l'esecuzione del processo corrente
    SYSCALL(TERMPROCESS, 0, 0, 0); 
}

// SYS3
void write_to_printer (state_t *exception_state, int asid) {
    // Stringa da scrivere
    char *s = (char *) exception_state->reg_a1; 
    // Lunghezza della stringa da scrivere
    int len = exception_state->reg_a2; 
    
	// Ricavo l'indirizzo del device register associato alla stampante associata all'asid passato come parametro
	memaddr dev_reg_addr = (memaddr) (DEVREGSTRT_ADDR + ((PRNTINT - 3) * 0x80) + (asid * 0x10));
    devreg_t *dev_reg = (devreg_t *) dev_reg_addr;

    // Errore, lunghezza non valida / indirizzo non valido
    if (len < 0 || len > MAXSTRLENG || (memaddr) s < KUSEG) terminate(asid); 

    SYSCALL(PASSEREN, (memaddr) &printer_sem[asid], 0, 0); 
    for (int i = 0; i < len; i++){
        dev_reg->dtp.data0 = s[i]; 
        int status = SYSCALL(DOIO, (memaddr) &(dev_reg->dtp.command), TRANSMITCHAR, 0); 
        if (status != READY){
            SYSCALL(VERHOGEN, (memaddr) &printer_sem[asid], 0, 0); 
            exception_state->reg_v0 = -status; 
            return; 
        }
    }

    SYSCALL(VERHOGEN, (memaddr) &printer_sem[asid], 0, 0); 
    exception_state->reg_v0 = len;    
}

// SYS4
void write_to_terminal (state_t *exception_state, int asid) {
    // Stringa da scrivere
    char *s = (char *) exception_state->reg_a1; 
    // Lunghezza della stringa da scrivere
    int len = exception_state->reg_a2; 
    
	// Ricavo l'indirizzo del device register associato alla terminale dell'asid passato come parametro
	memaddr dev_reg_addr = (memaddr) (DEVREGSTRT_ADDR + ((TERMINT - 3) * 0x80) + (asid * 0x10));
    devreg_t *dev_reg = (devreg_t *) dev_reg_addr;

    // Errore, lunghezza non valida / indirizzo non valido
    if (len < 0 || len > MAXSTRLENG || (memaddr) s < KUSEG) terminate(asid);

    SYSCALL(PASSEREN, (memaddr) &twrite_sem[asid], 0, 0); 
    for (int i = 0; i < len; i++){
        int status = SYSCALL(DOIO, (memaddr) &(dev_reg->term.transm_command), TRANSMITCHAR | (s[i] << 8), 0); 
        if ((status & TERMSTATMASK) != RECVD){
            SYSCALL(VERHOGEN, (memaddr) &twrite_sem[asid], 0, 0); 
            exception_state->reg_v0 = -status; 
            return; 
        }
    }

    SYSCALL(VERHOGEN, (memaddr) &twrite_sem[asid], 0, 0); 
    exception_state->reg_v0 = len;   
}

// SYS5
void read_from_terminal (state_t *exception_state, int asid) {
    char *buffer = (char *) exception_state->reg_a1;
    int transmitted = 0;
    char c = EOS;

    // Ricavo l'indirizzo del device register associato al terminale dell'asid passato come parametro
	memaddr dev_reg_addr = (memaddr) (DEVREGSTRT_ADDR + ((TERMINT - 3) * 0x80) + (asid * 0x10)); 
    devreg_t *dev_reg = (devreg_t *) dev_reg_addr;

    if ((memaddr) buffer < KUSEG) terminate(asid);

    SYSCALL(PASSEREN, (memaddr) &tread_sem[asid],0,0); 
    while (c != '\n') {
        int status = SYSCALL(DOIO, (int)&dev_reg->term.recv_command, (int) TRANSMITCHAR, 0);
        if ((status & TERMSTATMASK) != RECVD){
            SYSCALL(VERHOGEN, (memaddr) &tread_sem[asid],0,0); 
            exception_state->reg_v0 = -(status & TERMSTATMASK); 
            return; 
        }
        c = (char) ((status >> 8) & TERMSTATMASK); 
        buffer[transmitted++] = c;
    }
    SYSCALL(VERHOGEN, (memaddr) &tread_sem[asid],0,0); 
    exception_state->reg_v0 = transmitted; 
}
