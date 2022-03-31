#include "../h/exceptions.h"

cpu_t exception_time; 
void exception_handler(){
    STCK(exception_time); 

    /* Recupero dello stato al momento dell'eccezione del processore */
    exception_state = BIOSDATAPAGE; 
    
    /* And bitwise per estrarre il Cause.ExcCode */
    int cause = exception_state->cause & GETEXECCODE; 

    /* Shift per migliore manipolazione */
    cause = cause >> 2;

    /* Switch per la gestione dell'eccezione */
    switch(cause){
        case IOINTERRUPTS:                                  /* Si è verificato un interrupt */
            interrupt_handler(); 
            break; 
        case 1:                                             /* Si è verificata una eccezione TLB*/ 
        case TLBINVLDL:
        case TLBINVLDS:
            pass_up_or_die(PGFAULTEXCEPT,exception_state); 
            break; 
        case SYSEXCEPTION:                                  /* E' stata chiamata una system call */
            if (!(exception_state->status & USERPON))       /* La chiamata è avvenuta in kernel mode */
                syscall_handler(); 
            else                                            /* La chiamata non è avvenuta in kernel mode */
                pass_up_or_die(GENERALEXCEPT,exception_state); 
            break;
        default:                                            /* E' scattata una trap */
            pass_up_or_die(GENERALEXCEPT,exception_state); 
            break; 
    }

}

void syscall_handler(){
    /* Intero che rappresenta il tipo di system call */
    int syscode = exception_state->reg_a0;
    /* Intero che può assumere due valori: 1 se la system call è bloccante, 0 altrimenti */
    int block_flag = 0; 
    /* Intero che può assumere due valori: 1 se il processo corrente è stato ucciso dalla SYSC2, 0 altrimenti */
    int curr_proc_killed = 0; 
    
    /* Switch per la gestione della syscall */
    switch(syscode){
        case CREATEPROCESS:
            state_t *a1_state = (state_t *) exception_state->reg_a1; 
            int a2_p_prio = (int) exception_state->reg_a2; 
            support_t *a3_p_support_struct = (support_t *) exception_state->reg_a3; 

            create_process(a1_state, a2_p_prio, a3_p_support_struct); 
            break; 
        case TERMPROCESS:
            /* PID del processo chiamante */
            int a2_pid = exception_state->reg_a2; 

            terminate_process(a2_pid); 
            /* Il processo corrente e' stato terminato , dovrà essere scelto un nuovo current_p da eseguire */
            if (current_p == NULL)  curr_proc_killed = 1; 
            break; 
        case PASSEREN:
            int *a1_semaddr = exception_state->reg_a1;
            passeren(a1_semaddr, &block_flag);
            break; 
        case VERHOGEN:
            int *a1_semaddr = exception_state->reg_a1;
            verhogen(a1_semaddr);
            break; 
        case DOIO:
            int *a1_cmdAddr = exception_state->reg_a1;
            int a2_cmdValue = exception_state->reg_a2;
            block_flag = 1; 
            do_io(a1_semaddr, a2_cmdValue, &block_flag);
            break; 
        case GETTIME:
            get_cpu_time(&block_flag);
            break; 
        case CLOCKWAIT:
            block_flag = 1;
            wait_for_clock(&block_flag);
            break; 
        case GETSUPPORTPTR:
            get_support_data();
            break; 
        case GETPROCESSID:
            int a1_parent = exception_state->reg_a1;
            get_processor_id(a1_parent);
            break; 
        case YIELD:
            yield();
            break; 
    }


    /* Aggiornamento PC per evitare loop */
    exception_state->pc_epc += WORDLEN; 

    /* La syscall è bloccante / il processo corrente è stato ucciso */
    if (block_flag == 1 || curr_proc_killed == 1){
        if (curr_proc_killed == 0){                                         /* Se current_p è terminato (ovvero current_p == NULL), bisogna direttamente chiamare lo scheduler */
            /* 
                La syscall è bloccante => bisogna assegnare allo stato del processo corrente, 
                lo stato che si trova memorizzato all'inizio del BIOS Data Page 
            */
            current_p->p_s.entry_hi = exception_state->entry_hi;
            current_p->p_s.cause = exception_state->cause;
            current_p->p_s.status = exception_state->status;
            current_p->p_s.pc_epc = exception_state->pc_epc;
            for (int i = 0; i < STATE_GPR_LEN; i++)
                current_p->p_s.gpr[i] = exception_state->gpr[i];
            current_p->p_s.hi = exception_state->hi;
            current_p->p_s.lo = exception_state->lo;

            /* Se la syscall è bloccante, il tempo accumulato sulla CPU del processo, deve essere aggiornato */
            current_p->p_time = exception_time - start_usage_cpu;
        }
        /* 
            Infine, viene chiamato lo scheduler perchè current_p è stato bloccato 
            dalla syscall / è terminato => bisogna scegliere un nuovo current_p da eseguire
        */
        scheduler(); 
    } else {                        /* syscall non bloccanti */
        /* Si carica in memoria lo stato del processo al momento dell'eccezione e si continua l'esecuzione di current_p */
        LDST(exception_state); 
    }

}

void create_process(state_t *a1_state, int a2_p_prio, support_t *a3_p_support_struct){
    /* Tentativo di allocazione di un nuovo processo */
    pcb_PTR new_proc = allocPcb(); 

    if (new_proc != NULL){                           /* Allocazione avvenuta con successo */
        /* Il nuovo processo è figlio del processo chiamante */
        insertChild(current_p,new_proc); 

        /* Inizializzazione campi del nuovo processo a partire da a1,a2,a3 */
        new_proc->p_s = *a1_state; 
        new_proc->p_prio = a2_p_prio; 
        new_proc->p_supportStruct = a3_p_support_struct; 

        /* PID è implementato come l'indirizzo del pcb_t */
        new_proc->p_pid = new_proc; 


        /* Il nuovo processo è pronto per essere messo nella ready_q */
        switch(new_proc->p_prio){
            case PROCESS_PRIO_LOW:                                  /* E' un processo a bassa priorità */
                insertProcQ(&(ready_lq->p_list),new_proc); 
                break;
            default:                                 /* E' un processo ad alta priorità */
                insertProcQ(&(ready_hq->p_list),new_proc); 
                break; 
        }

        /* Operazione completata, ritorno con successo */
        current_p->p_s.reg_v0 = new_proc->p_pid; 
    }else                                           /* Allocazione fallita, risorse non disponibili */
        current_p->p_s.reg_v0 = NOPROC; 

}

void terminate_process(int a2_pid){
    pcb_PTR old_proc; 
    if (a2_pid == 0){                                      
        /* Rimozione di current_p dalla lista dei figli del suo padre */
        outChild(current_p); 
        /* Terminazione di tutta la discendenza di current_p*/
        if (current_p != NULL) terminate_all(current_p); 
        /* Terminazione del processo corrente */
        current_p = NULL; 
    }else{
        /* PID è implementato come indirizzo del PCB */
        old_proc = a2_pid; 
        /* Rimozione di old_proc dalla lista dei figli del suo padre */
        outChild(old_proc); 
        /* Terminazione di tutta la discendenza di old_proc */
        if (old_proc != NULL) terminate_all(old_proc); 
    }
}

void terminate_all(pcb_PTR old_proc){
    if (old_proc != NULL){
        pcb_PTR child; 
        while(child = removeChild(old_proc))
            terminate_all(child); 

        /* 
            Task di terminazione di un processo (sezione 3.9, manuale pandosplus) 
        */


        /* Aggiornamento semafori / variabile di conteggio dei bloccati su I/O */
        if (old_proc->p_semAdd != NULL)
            if ((old_proc->p_semAdd >= &(sem[0])) && (old_proc->p_semAdd <= &(sem[DEVICE_EXCEPTIONS])))
                soft_counter -= 1;
            else
                *(old_proc->p_semAdd) += 1; 
        else{
            /* Rimozione dalla coda dei processi ready */
            switch (old_proc->p_prio){
            case PROCESS_PRIO_LOW:
                outProcQ(&(ready_lq->p_list), old_proc);
                break;
            default:
                outProcQ(&(ready_hq->p_list), old_proc);
                break;
            }
        }
        p_count -= 1;
        /* Inserimento di old_proc nella lista dei pcb liberi, da allocare */
        freePcb(old_proc); 
    }
}

void passeren (int *a1_semaddr, int *block_flag) {
    *a1_semaddr -= 1;

    if (a1_semaddr < 0) {
        if (insertBlocked(a1_semaddr,current_p))            /* Se non ci sono semafori liberi, PANIC */
            PANIC(); 
        else
            scheduler();                                    /* Il processo corrente si è bloccato, viene scelto un altro da eseguire */
    } else {
        block_flag = 0;                                     /* L'esecuzione ritorna al processo corrente */
    }
}

void verhogen (int *a1_semaddr) {
    *a1_semaddr += 1;
    /* Rimozione del primo pcb dalla coda dei processi bloccati su a1_semaddr */
    pcb_PTR unblocked_p = removeBlocked(a1_semaddr);
    if (unblocked_p != NULL){                                   /* Se è stato "sbloccato" un processo */
        /* Inserimento nella ready queue in base alla priorità */
        switch(unblocked_p->p_prio){
            case PROCESS_PRIO_LOW:
                insertProcQ(&(ready_lq->p_list),unblocked_p); 
                break; 
            default:
                insertProcQ(&(ready_hq->p_list),unblocked_p); 
                break; 
        }
    }

}

void do_io(int *a1_cmdAddr, int a2_cmdValue, int *block_flag) {
}

void get_cpu_time() {
    /* 
        Quando l'exception_handler viene richiamato, viene memorizzato il TOD corrente in "exception_time". 
        "start_usage_cpu" è il TOD al momento dell'assegnamento della CPU al processo corrente da parte dello scheduler.
    */
    exception_state->reg_v0 = current_p->p_time + (exception_time - start_usage_cpu);
}

void wait_for_clock(int *block_flag) {
    passeren(sem[INTERVAL_INDEX], &block_flag);
    block_flag = 0;
}

support_t* get_support_data() {
    exception_state->reg_v0 = current_p->p_supportStruct;
}

void get_processor_id(int a1_parent) {
    if (a1_parent == 0)
        exception_state->reg_v0 = current_p->p_pid;
    else
        exception_state->reg_v0 = (current_p->p_parent)->p_pid;
}

void yield() {
    /* Switch per agire sulle code in base alla priorità del processo */
    switch(current_p->p_prio){
        case PROCESS_PRIO_LOW:
            outProcQ(&(ready_lq->p_list),current_p); 
            insertProcQ(&(ready_lq->p_list),current_p);
            /* 
                Poichè il processo che ha ceduto la CPU è a bassa priorità, la scelta del nuovo 
                processo da eseguire può essere fatta semplicemente dallo scheduler.
            */
            scheduler(); 
            break; 
        default:
            outProcQ(&(ready_hq->p_list),current_p); 
            insertProcQ(&(ready_hq->p_list),current_p); 
            /*
                Il processo corrente ha "ceduto" il controllo della CPU agli altri processi.
                A seguire è implementata la scelta del nuovo processo a cui cedere la CPU.
            */

            pcb_PTR next_hproc = headProcQ(&(ready_hq->p_list)); 
            pcb_PTR next_lproc = headProcQ(&(ready_lq->p_list)); 
            
            if (next_hproc != current_p)                        /* Lo scheduler sceglierà un altro processo da eseguire */
                scheduler(); 
            else if (next_lproc != NULL){
                /* Il nuovo processo da eseguire è un processo a bassa priorità */
                current_p = removeProcQ(&(ready_lq->p_list));
                /* Aggiornamento dello status register del processore al nuovo stato del nuovo processo scelto */
                LDST(&(current_p->p_s));
            }
            break; 
    }
}

void pass_up_or_die(int index_value, state_t* exception_state) {
    if (current_p->p_supportStruct == NULL) {           /* Se il processo non ha specificato un modo per gestire l'eccezione, viene terminato*/
        terminate_process(exception_state->reg_a2);
    } else {                                            /* Altrimenti, si "passa" la gestione dell'eccezione al passupvector della support struct*/
        (current_p->p_supportStruct)->sup_exceptState[index_value] = *exception_state; 
        context_t new_context = (current_p->p_supportStruct)->sup_exceptContext[index_value];
        LDCXT(new_context.stackPtr, new_context.status, new_context.pc); 
    }

}
