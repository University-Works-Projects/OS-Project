#include "../h/exceptions.h"

extern int sem[DEVICE_INITIAL];                         /* Semafori associati ai dispositivi */  
extern cpu_t start_usage_cpu; 
state_t *exception_state;                               /* Stato del processore al momento dell'eccezione */
extern pcb_PTR current_p;                               /* Processo responsabile dell'eccezione */
extern struct list_head ready_hq;                       /* Ready queues */
extern struct list_head ready_lq; 
extern int p_count;                                     /* Processi vivi */
extern int soft_counter;                                /* Processi bloccati, che stanno aspettando una operazione di I/O */
extern void scheduler(); 

cpu_t exception_time; 
state_t *exception_state; 

/* Gestore delle eccezioni */
void exception_handler() {
    STCK(exception_time); 

    exception_state = (state_t *) BIOSDATAPAGE;             /* Recupero dello stato al momento dell'eccezione del processore */
    int syscode; 
    int cause = exception_state->cause & GETEXECCODE;       /* And bitwise per estrarre il Cause.ExcCode */

    cause = cause >> CAUSESHIFT;                                     /* Shift per migliore manipolazione */

    switch(cause) {                                         /* Switch per la gestione dell'eccezione */
        case IOINTERRUPTS:                                      /* Si è verificato un interrupt */
            interrupt_handler(exception_state); 
            break; 
        case 1:                                                 /* Si è verificata una eccezione TLB */ 
        case TLBINVLDL:
        case TLBINVLDS:
            pass_up_or_die(PGFAULTEXCEPT, exception_state); 
            break; 
        case SYSEXCEPTION:                                      /* E' stata chiamata una system call */
            syscode = exception_state->reg_a0;                  /* Intero che rappresenta il tipo di system call */
            if ((!(exception_state->status & USERPON) && syscode < 0 ) || ((exception_state->status & USERPON) && syscode > 0))           /* La chiamata è avvenuta in kernel mode */
                syscall_handler(); 
            else {                                              /* La chiamata non è avvenuta in kernel mode */
                exception_state->cause = PRIVINSTR << CAUSESHIFT; 
                pass_up_or_die(GENERALEXCEPT, exception_state); 
            }
            break;
        default:                                                /* E' scattata una trap */
            pass_up_or_die(GENERALEXCEPT, exception_state); 
            break; 
    }
}

/* Gestore delle syscall */
void syscall_handler() {
    int syscode = exception_state->reg_a0;                  /* Intero che rappresenta il tipo di system call */
    int block_flag = 0;                                     /* block_flag = c'è bisogno di chiamare lo scheduler ? 1 : 0 */
    int low_priority = 0;                                   /* low_priority = the next pcb to run is to be taken from ready_lq ? 1 : 0 */
    int curr_proc_killed = 0;                               /* curr_proc_killed = current_p was killed from SYSC2 ? 1 : 0 */
    
    switch(syscode) {                                       /* Switch per la gestione della syscall */
        case CREATEPROCESS:
            {
                state_t *a1_state = (state_t *) exception_state->reg_a1; 
                int a2_p_prio = (int) exception_state->reg_a2; 
                support_t *a3_p_support_struct = (support_t *) exception_state->reg_a3; 

                create_process(a1_state, a2_p_prio, a3_p_support_struct); 
            }
            break; 
        case TERMPROCESS:
            {
                int a1_pid = exception_state->reg_a1;                       /* PID del processo chiamante */ 

                terminate_process(a1_pid); 
            }
            if (current_p == NULL) curr_proc_killed = 1;                    /* current_p è morto, dovrà essere scelto un nuovo current_p da eseguire */ 
            break; 
        case PASSEREN:
            {
                int *a1_semaddr = (int *) exception_state->reg_a1;
                sem_operation(a1_semaddr, &block_flag,1);
            }
            break; 
        case VERHOGEN:
            {
                int *a1_semaddr = (int *) exception_state->reg_a1;
                sem_operation(a1_semaddr, &block_flag,0);
            }
            break; 
        case DOIO:
            {
                int *a1_cmdAddr = (int *) exception_state->reg_a1;
                int a2_cmdValue = exception_state->reg_a2;
                do_io(a1_cmdAddr, a2_cmdValue, &block_flag);
            }
            break; 
        case GETTIME:
            get_cpu_time(&block_flag);
            break; 
        case CLOCKWAIT:
            wait_for_clock(&block_flag);
            break; 
        case GETSUPPORTPTR:
            get_support_data();
            break; 
        case GETPROCESSID:
            {
                int a1_parent = exception_state->reg_a1;
                get_processor_id(a1_parent);
            }
            break; 
        case YIELD:
            yield(&block_flag, &low_priority);
            break; 
        default:
            pass_up_or_die(GENERALEXCEPT, exception_state); 
            break; 
    }
    if (syscode != DOIO && syscode < 0) {
        if (curr_proc_killed == 0) {
            exception_state->pc_epc += WORDLEN;                                 /* Aggiornamento PC per evitare loop */
            copy_state(&(current_p->p_s), exception_state);
            current_p->p_time += exception_time - start_usage_cpu;
            STCK(start_usage_cpu); 
        }
        if (low_priority == 0) {
            if (block_flag == 1 || curr_proc_killed == 1) {
                current_p = NULL; 
                scheduler();
            } else
                LDST(&(current_p->p_s));
        } else {
            STCK(start_usage_cpu); 
            current_p = removeProcQ(&(ready_lq));                               /* Il nuovo processo da eseguire è un processo a bassa priorità */
            LDST(&(current_p->p_s));                                            /* Aggiornamento dello status register del processore al nuovo stato del nuovo current_p */
        }
    }
}

/* NSYS1 */
void create_process(state_t *a1_state, int a2_p_prio, support_t *a3_p_support_struct) {
    pcb_PTR new_proc = allocPcb();                                  /* Tentativo di allocazione di un nuovo processo */

    if (new_proc != NULL) {                                         /* Allocazione avvenuta con successo */
        insertChild(current_p, new_proc);                           /* Il nuovo processo è figlio del processo chiamante */

        /* Inizializzazione campi del nuovo processo a partire da a1, a2, a3 */
        copy_state(&(new_proc->p_s), a1_state); 
        new_proc->p_prio = a2_p_prio; 
        new_proc->p_supportStruct = a3_p_support_struct; 

        new_proc->p_pid = (int) new_proc;                           /* PID è implementato come l'indirizzo del pcb_t */
        p_count++; 
        ready_by_priority(new_proc); 
        exception_state->reg_v0 = new_proc->p_pid;                  /* Operazione completata, ritorno con successo */
    } else
        exception_state->reg_v0 = NOPROC;                           /* Allocazione fallita, risorse non disponibili */
}

/* NSYS2 */
void terminate_process(int a1_pid) {
    pcb_PTR old_proc; 
    if (a1_pid == 0) {
        outChild(current_p);                                /* Rimozione di current_p dalla lista dei figli del suo padre */
        if (current_p != NULL) 
            terminate_all(current_p);                       /* Terminazione di tutta la discendenza di current_p */ 
        current_p = NULL;                                    /* Terminazione del processo corrente */
    } else {
        old_proc = (pcb_PTR) a1_pid;                        /* PID è implementato come indirizzo del PCB */
        outChild(old_proc);                                 /* Rimozione di old_proc dalla lista dei figli del suo padre */ 
        if (old_proc != NULL) 
            terminate_all(old_proc);      /* Terminazione di tutta la discendenza di old_proc */
        if (old_proc == current_p)
            current_p = NULL; 
    }
}

/* Funzione ausiliaria ricorsiva che termina l'intera discendenza del processo old_proc (incluso old_proc) */
void terminate_all(pcb_PTR old_proc) {
    if (old_proc != NULL) {
        pcb_PTR child;   
        while((child = removeChild(old_proc)) != NULL){      /* Task di terminazione di un processo (sezione 3.9, manuale pandosplus) */
            if (child == current_p)
                current_p = NULL; 
            terminate_all(child); 
        }

        /* Aggiornamento semafori / variabile di conteggio dei bloccati su I/O */
        if (old_proc->p_semAdd != NULL) {
            if ((old_proc->p_semAdd >= &(sem[0])) && (old_proc->p_semAdd <= &(sem[DEVICE_INITIAL])))
                soft_counter -= 1;
            outBlocked(old_proc); 
        }
        switch (old_proc->p_prio) {
            case PROCESS_PRIO_LOW:
                outProcQ(&(ready_lq), old_proc);
                break;
            default:
                outProcQ(&(ready_hq), old_proc);
                break;
        }
        p_count -= 1;
        freePcb(old_proc);                                  /* Inserimento di old_proc nella lista dei pcb liberi da allocare */ 
    }
}

/* NSYS3 & NSYS4*/
void sem_operation(int *a1_semaddr, int *block_flag, int p_flag) {
    if ((*a1_semaddr == 0 && p_flag) || (*a1_semaddr == 1 && !p_flag)) {
        if (insertBlocked(a1_semaddr, current_p))         /* Se non ci sono semafori liberi, PANIC */
            PANIC();
        *block_flag = 1; 
        if ((a1_semaddr >= &sem[0]) && (a1_semaddr <= &sem[DEVICE_INITIAL]))
            soft_counter++;
    } else if (headBlocked(a1_semaddr) == NULL){
        if (p_flag) *a1_semaddr = 0; 
        else        *a1_semaddr = 1; 
        *block_flag = 0;                                    /* L'esecuzione ritorna al processo corrente */
    } else{
        pcb_PTR unblocked_p = removeBlocked(a1_semaddr);                /* Rimozione del primo pcb dalla coda dei processi bloccati su a1_semaddr */
        unblocked_p->p_semAdd = NULL; 
        *block_flag = 0; 
        ready_by_priority(unblocked_p); 
        if ((a1_semaddr >= &sem[0]) && (a1_semaddr <= &sem[DEVICE_INITIAL]))
            soft_counter--;
    }
}


/* NSYS5 */
void do_io(int *a1_cmdAddr, int a2_cmdValue, int *block_flag) {
    int line_size = (DEVPERINT * DEVREGSIZE);                                               /* Dimenzione di una linea */
    int line_no = (((unsigned int) a1_cmdAddr - DEVREGSTRT_ADDR) / (line_size)) + 3;        /* Numero della linea */
    int dev_reg_start_addr = ((line_no - 3) * (line_size) + DEVREGSTRT_ADDR);               /* Indirizzo di inizio dei device register della linea line */
    int device_no = ((unsigned int) a1_cmdAddr - dev_reg_start_addr)/ DEVREGSIZE;           /* Numero del device */
    int device_index = (line_no - 3) * 8 + device_no + 1;                                   /* Indice del device semaphore */
    if (line_no == TERMINT && ((unsigned int) a1_cmdAddr - (dev_reg_start_addr) + device_no * DEVREGSIZE) < 0x8)    /* Controllo, se si tratta della linea dei terminali, a quale sub-device ci si riferisce: recv o trasm */
        device_index += DEVPERINT;

    sem_operation(&sem[device_index], block_flag,1); 
    *a1_cmdAddr = a2_cmdValue;                                  /* Aggiornamento PC per evitare loop */
    exception_state->pc_epc += WORDLEN;
    copy_state(&(current_p->p_s), exception_state);
    current_p->p_time += exception_time - start_usage_cpu;
    STCK(start_usage_cpu); 
    current_p = NULL;
    scheduler();
}

/* NSYS6 */
void get_cpu_time() {
   /* Quando l'exception_handler viene richiamato, viene memorizzato il TOD corrente in "exception_time".
      "start_usage_cpu" è il TOD al momento dell'assegnamento della CPU al processo corrente da parte dello scheduler. */
    exception_state->reg_v0 = current_p->p_time + (exception_time - start_usage_cpu);
}

/* NSYS7 */
void wait_for_clock(int *block_flag) {
    sem_operation(&sem[INTERVAL_INDEX], block_flag,1);
}

/* NSYS8 */
void get_support_data() {
    exception_state->reg_v0 = (memaddr) current_p->p_supportStruct;
}

/* NSYS9 */
void get_processor_id(int a1_parent) {
    if (a1_parent == 0)
        exception_state->reg_v0 = (unsigned int) current_p;
    else
        exception_state->reg_v0 = (unsigned int) current_p->p_parent;
}

/* NSYS10 */
void yield(int *block_flag, int *low_priority) {
    switch(current_p->p_prio) {                                 /* Switch per agire sulle code in base alla priorità del processo */
        case PROCESS_PRIO_LOW:
            outProcQ(&(ready_lq), current_p); 
            insertProcQ(&(ready_lq), current_p);
            /* Poichè il processo che ha ceduto la CPU è a bassa priorità, la scelta del nuovo 
               processo da eseguire può essere fatta semplicemente dallo scheduler. */
            *block_flag = 1; 
            break; 
        default:
            outProcQ(&(ready_hq), current_p); 
            insertProcQ(&(ready_hq), current_p); 
            /* Il processo corrente ha "ceduto" il controllo della CPU agli altri processi.
               A seguire è implementata la scelta del nuovo processo a cui cedere la CPU. */

            pcb_PTR next_hproc = headProcQ(&(ready_hq)); 
            pcb_PTR next_lproc = headProcQ(&(ready_lq)); 
            
            if (next_hproc != current_p)                        /* Lo scheduler sceglierà un altro processo da eseguire */
                *block_flag = 1; 
            else if (next_lproc != NULL)
                *low_priority = 1; 
            break; 
    }
}

/* Program Trap handler & TLB Exception handler */
void pass_up_or_die(int index_value, state_t* exception_state) {
    if (current_p->p_supportStruct == NULL) {           /* Se il processo non ha specificato un modo per gestire l'eccezione, viene terminato*/
        terminate_process(0);
        current_p = NULL; 
        scheduler(); 
    } else {                                            /* Altrimenti, si "passa" la gestione dell'eccezione al passupvector della support struct*/
        copy_state(&((current_p->p_supportStruct)->sup_exceptState[index_value]), exception_state); 
        context_t new_context = (current_p->p_supportStruct)->sup_exceptContext[index_value];
        LDCXT(new_context.stackPtr, new_context.status, new_context.pc); 
    }
}

/* Utility function */
void ready_by_priority(pcb_PTR to_insert){
    if (to_insert != NULL)
        switch(to_insert->p_prio) {                               /* Inserimento nella ready queue in base alla priorità */
            case PROCESS_PRIO_LOW:
                insertProcQ(&(ready_lq), to_insert); 
                break; 
            default:
                insertProcQ(&(ready_hq), to_insert); 
                break; 
        }
}

/* TLB-Refill Handler */
void uTLB_RefillHandler() {
    // Recupero dello stato al momento dell'eccezione del processore
    exception_state = (state_t *) BIOSDATAPAGE;

    // Recupero del numero di pagina che non si trova nel TLB
    int page_missing = (exception_state->entry_hi - KUSEG) >> VPNSHIFT; 

    if ((exception_state->entry_hi >> VPNSHIFT) == 0xBFFFF)
        // Si tratta della pagina dello stack
        page_missing = MAXPAGES - 1;

    // Scrittura della entry in TLB
    setENTRYHI(current_p->p_supportStruct->sup_privatePgTbl[page_missing].pte_entryHI);
    setENTRYLO(current_p->p_supportStruct->sup_privatePgTbl[page_missing].pte_entryLO);
    TLBWR();

    // Riprova l'ultima istruzione che ha causato l'eccezione TLB-Refill
    LDST(exception_state);
}