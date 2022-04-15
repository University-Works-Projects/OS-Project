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

/**
 * The exception handler is called when an exception occurs. It determines the cause of the exception
 * and calls the appropriate handler. 
 * 
 * The first thing the exception handler does is save the current time in the variable exception_time.
 * This is used to calculate the time spent in the exception handler. 
 * 
 * The exception handler then gets the current state of the processor from the BIOS data page. 
 * 
 * The cause of the exception is then determined by masking the cause register with the GETEXECCODE
 * macro. The cause is then shifted right by two bits to get the exception code. 
 * 
 * The exception code is then used to determine the cause of the exception. 
 */
void exception_handler() {
    STCK(exception_time); 

    exception_state = (state_t *) BIOSDATAPAGE;             /* Recupero dello stato al momento dell'eccezione del processore */
    
    int cause = exception_state->cause & GETEXECCODE;       /* And bitwise per estrarre il Cause.ExcCode */

    cause = cause >> 2;                                     /* Shift per migliore manipolazione */

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
            if (!(exception_state->status & USERPON))           /* La chiamata è avvenuta in kernel mode */
                syscall_handler(); 
            else {                                              /* La chiamata non è avvenuta in kernel mode */
                exception_state->cause = PRIVINSTR; 
                pass_up_or_die(GENERALEXCEPT, exception_state); 
            }
            break;
        default:                                                /* E' scattata una trap */
            pass_up_or_die(GENERALEXCEPT, exception_state); 
            break; 
    }
}

/**
 * The function handles the system calls, updating the current_p and calling the scheduler if
 * necessary. 
 * 
 * The function extracts the system call code from the exception state and calls the appropriate
 * function. 
 * 
 * The function also handles the case in which the current_p is killed by the system call. In this
 * case, the function sets the curr_proc_killed flag to 1 and calls the scheduler. 
 * 
 * The function also handles the case in which the current_p is blocked by the system call. In this
 * case, the function sets the block_flag to 1 and calls the scheduler. 
 * 
 * The function also handles the case in which the current_p is not blocked or killed by the system
 * call. In this case, the function updates the current_p's state and calls the scheduler. 
 */
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
                b_passeren(a1_semaddr, &block_flag);
            }
            break; 
        case VERHOGEN:
            {
                int *a1_semaddr = (int *) exception_state->reg_a1;
                b_verhogen(a1_semaddr, &block_flag);
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

/**
 * It creates a new process, initializes its fields and inserts it in the ready queue
 * 
 * @param a1_state the state of the process to be created
 * @param a2_p_prio the priority of the new process.
 * @param a3_p_support_struct a pointer to a support_t struct. This struct is used to store the
 * semaphore and the device semaphore that the process is waiting on.
 */
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

        switch(new_proc->p_prio) {                                  /* Il nuovo processo è pronto per essere messo nella ready_q */
            case PROCESS_PRIO_LOW:                                  /* E' un processo a bassa priorità */
                insertProcQ(&(ready_lq), new_proc); 
                break;
            default:                                                /* E' un processo ad alta priorità */
                insertProcQ(&(ready_hq), new_proc); 
                break; 
        }
        exception_state->reg_v0 = new_proc->p_pid;                  /* Operazione completata, ritorno con successo */
    } else
        exception_state->reg_v0 = NOPROC;                           /* Allocazione fallita, risorse non disponibili */
}

/**
 * It removes the process from the list of children of its parent, and then it terminates all its
 * descendants
 * 
 * @param a2_pid The process ID of the process to terminate.
 */
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

/**
 * It removes all the children of the process passed as argument, and then it frees the process itself
 * 
 * @param old_proc the process to be terminated
 */
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

/**
 * If a1_semaddr is 0, it try to insert the pcb into the semaphore's queue;
 * If a1_semaddr is 1 and there are processes blocked on the semaphore, free the oldest one; otherwise,
 * the process continues its execution
 * 
 * @param a1_semaddr the address of the semaphore to be passed
 * @param block_flag a flag that indicates whether the current process has been blocked or not.
 */
void b_passeren (int *a1_semaddr, int *block_flag) {
    if (*a1_semaddr == 0) {
        if (insertBlocked(a1_semaddr, current_p))         /* Se non ci sono semafori liberi, PANIC */
            PANIC();
        *block_flag = 1; 
        if ((a1_semaddr >= &sem[0]) && (a1_semaddr <= &sem[DEVICE_INITIAL]))
            soft_counter++;
    } else if (headBlocked(a1_semaddr) == NULL){
        *a1_semaddr = 0; 
        *block_flag = 0;                                    /* L'esecuzione ritorna al processo corrente */
    } else{
        pcb_PTR unblocked_p = removeBlocked(a1_semaddr);                /* Rimozione del primo pcb dalla coda dei processi bloccati su a1_semaddr */
        unblocked_p->p_semAdd = NULL; 
        *block_flag = 0; 
        switch(unblocked_p->p_prio) {                               /* Inserimento nella ready queue in base alla priorità */
            case PROCESS_PRIO_LOW:
                insertProcQ(&(ready_lq), unblocked_p); 
                break; 
            default:
                insertProcQ(&(ready_hq), unblocked_p); 
                break; 
        }
        if ((a1_semaddr >= &sem[0]) && (a1_semaddr <= &sem[DEVICE_INITIAL]))
            soft_counter--;
    }
}

/**
 * The function b_verhogen() increments the semaphore value and removes the first process from the
 * blocked queue of the semaphore if *a1_semaddr is 0. If a process is removed, it is inserted in the ready queue according
 * to its priority.
 * If *a1_semaddr is 1, b_verhogen acts as a blocking-syscall, inserting the curren_p into the list of blocked processes in a1_semaddr.
 * 
 * The function b_verhogen() is called by the interrupt handler of the device, when the device is ready
 * to accept a new request.
 * 
 * @param a1_semaddr the address of the semaphore to be incremented
 * @param block_flag a flag that indicates whether the current process has been blocked or not.
 * 
 * @return The first process in the blocked queue of the semaphore.
 */
pcb_PTR b_verhogen (int *a1_semaddr, int *block_flag) {
    if (*a1_semaddr == 1){
        if (insertBlocked(a1_semaddr,current_p))
            PANIC(); 
        *block_flag = 1; 
        if ((a1_semaddr >= &sem[0]) && (a1_semaddr <= &sem[DEVICE_INITIAL]))
            soft_counter++;
        return NULL; 
    }else if (headBlocked(a1_semaddr) == NULL){
        *block_flag = 0; 
        *a1_semaddr = 1; 
        return NULL; 
    }else{
        pcb_PTR unblocked_p = removeBlocked(a1_semaddr);                /* Rimozione del primo pcb dalla coda dei processi bloccati su a1_semaddr */
        unblocked_p->p_semAdd = NULL; 
        *block_flag = 0; 
        switch(unblocked_p->p_prio) {                               /* Inserimento nella ready queue in base alla priorità */
            case PROCESS_PRIO_LOW:
                insertProcQ(&(ready_lq), unblocked_p); 
                break; 
            default:
                insertProcQ(&(ready_hq), unblocked_p); 
                break; 
        }
        if ((a1_semaddr >= &sem[0]) && (a1_semaddr <= &sem[DEVICE_INITIAL]))
            soft_counter--;
        return unblocked_p; 
    }
    return NULL; 
}

/**
 * It checks if the device is available, if so it performs the I/O operation, otherwise it blocks the
 * process
 * 
 * @param a1_cmdAddr the address of the device register to be accessed
 * @param a2_cmdValue the value to be written to the device register
 * @param block_flag if the process is blocked, it is set to 1, otherwise it is set to 0.
 */
void do_io(int *a1_cmdAddr, int a2_cmdValue, int *block_flag) {
    int line_size = (DEVPERINT * DEVREGSIZE);                                               /* Dimenzione di una linea */
    int line_no = (((unsigned int) a1_cmdAddr - DEVREGSTRT_ADDR) / (line_size)) + 3;        /* Numero della linea */
    int dev_reg_start_addr = ((line_no - 3) * (line_size) + DEVREGSTRT_ADDR);               /* Indirizzo di inizio dei device register della linea line */
    int device_no = ((unsigned int) a1_cmdAddr - dev_reg_start_addr)/ DEVREGSIZE;           /* Numero del device */
    int device_index = (line_no - 3) * 8 + device_no + 1;                                   /* Indice del device semaphore */
    if (line_no == TERMINT && ((unsigned int) a1_cmdAddr - (dev_reg_start_addr) + device_no * DEVREGSIZE) < 0x8)    /* Controllo, se si tratta della linea dei terminali, a quale sub-device ci si riferisce: recv o trasm */
        device_index += DEVPERINT;

    b_passeren(&sem[device_index], block_flag); 
    *a1_cmdAddr = a2_cmdValue;                                  /* Aggiornamento PC per evitare loop */
    exception_state->pc_epc += WORDLEN;
    copy_state(&(current_p->p_s), exception_state);
    current_p->p_time += exception_time - start_usage_cpu;
    STCK(start_usage_cpu); 
    current_p = NULL;
    scheduler();
}

/**
 * It returns the amount of time the process has been using the CPU
 */
void get_cpu_time() {
   /* Quando l'exception_handler viene richiamato, viene memorizzato il TOD corrente in "exception_time".
      "start_usage_cpu" è il TOD al momento dell'assegnamento della CPU al processo corrente da parte dello scheduler. */
    exception_state->reg_v0 = current_p->p_time + (exception_time - start_usage_cpu);
}

/**
 * It waits for the clock to tick
 * 
 * @param block_flag a pointer to a flag that indicates whether the process is blocked or not.
 */
void wait_for_clock(int *block_flag) {
    b_passeren(&sem[INTERVAL_INDEX], block_flag);
}

/**
 * It returns the address of the support structure of the current process
 */
void get_support_data() {
    exception_state->reg_v0 = (memaddr) current_p->p_supportStruct;
}

/**
 * If the parent is 0, return the current process, otherwise return the parent process.
 * 
 * @param a1_parent 0 if you want the current process, 1 if you want the parent process
 */
void get_processor_id(int a1_parent) {
    if (a1_parent == 0)
        exception_state->reg_v0 = (unsigned int) current_p;
    else
        exception_state->reg_v0 = (unsigned int) current_p->p_parent;
}

/**
 * The function `yield` is used to put the current process in the ready queue and to choose the next
 * process to be executed. 
 * 
 * @param block_flag if set to 1, the scheduler will be called to choose a new process to execute.
 * @param low_priority if set to 1, the scheduler will choose a low priority process to run.
 */
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

/**
 * If the current process has a support structure, then we copy the exception state into the support
 * structure's exception state array, and then we load the context of the exception handler into the
 * CPU
 * 
 * @param index_value the index of the exception in the exception vector
 * @param exception_state the state of the process when the exception occurred
 */
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