#include "../h/interrupts.h"

extern cpu_t exception_time; 
extern int sem[DEVICE_INITIAL];  
extern cpu_t start_usage_cpu; 
extern void copy_state(state_t *a, state_t *b); 
extern void scheduler(); 


void interrupt_handler(state_t* exception_state){
    /* Estrazione del campo IP dal registro CAUSE */
    int ip = exception_state->cause & IMON;

    /* La priorità delle chiamate è implementata in base all'ordine di attivazione dei seguenti if */
    if (ip & LOCALTIMERINT) {
        plt_handler(exception_state); 
    } else if (ip & TIMERINTERRUPT) {
        interval_handler(exception_state); 
    } else if (ip & DISKINTERRUPT) {
        non_timer_interrupt(DISKINT);     
    } else if (ip & FLASHINTERRUPT) { 
        non_timer_interrupt(FLASHINT);
    } else if (ip & NETINTERRUPT){
        non_timer_interrupt(NETWINT); 
    } else if (ip & PRINTINTERRUPT) { 
        non_timer_interrupt(PRNTINT); 
    } else if (ip & TERMINTERRUPT) { 
        non_timer_interrupt(TERMINT); 
    }
}

void plt_handler(state_t *exception_state){
    /* 
    Acknowledge dell'interrupt PLT: si scrive un nuovo valore nel registro Timer della CP0. 
    Grande abbastanza per permettere allo scheduler di scegliere un altro processo da eseguire e settare il PLT a TIMESLICE del nuovo processo da eseguire 
    */
    setTIMER(10000000);

    /* Salvataggio dello stato di esecuzione del processo al momento dell'interrupt */
    copy_state(&(current_p->p_s),exception_state); 
    /* Aggiornamento del tempo del processo */
    current_p->p_time += exception_time - start_usage_cpu;
    STCK(start_usage_cpu); 
    /* Inserimento del processo della ready queue */
    switch(current_p->p_prio){
        case PROCESS_PRIO_LOW:
            insertProcQ(&(ready_lq),current_p); 
            break; 
        default:
            insertProcQ(&(ready_hq),current_p); 
            break; 
    }
    current_p = NULL; 
    scheduler(); 
}

void interval_handler(state_t *exception_state){
    /* Acknowledge dell'interrupt dell'interval timer caricando un nuovo valore: 100ms */
    LDIT(100000); 
    
    /* Sblocco di tutti i pcb bloccati sul semaforo dell'interval timer */
    while(headBlocked(&(sem[INTERVAL_INDEX])) != NULL){
        pcb_PTR unblocked = verhogen(&(sem[INTERVAL_INDEX])); 
        if (unblocked != NULL){
            soft_counter--; 
        }
    } 
    /* Reset del semaforo a 0 così che le successive wait_clock() blocchino i processi */
    sem[INTERVAL_INDEX] = 0; 
    if (current_p == NULL) scheduler(); 
    else{
        /* Salvataggio dello stato di esecuzione del processo al momento dell'interrupt */
        copy_state(&(current_p->p_s), exception_state);
        /* Aggiornamento del tempo del processo */
        current_p->p_time += exception_time - start_usage_cpu;
        STCK(start_usage_cpu);
        /* Prosegue l'esecuzione del processo corrente */
        LDST(exception_state);
    }
}

void non_timer_interrupt(int line){
    memaddr *bitmap_word_addr = (memaddr *) ((BITMAPSTRT_ADDR) + (line - 3) * 0x04); 
    int device_interrupting = get_dev_interrupting(bitmap_word_addr);                                           /* Numero del device che ha provocato l'eccezione */
    memaddr dev_reg_addr = (memaddr) (DEVREGSTRT_ADDR + ((line - 3) * 0x80) + (device_interrupting * 0x10));    /* Inidirizzo del device register del device che ha provocato l'eccezione */
    devreg_t *dev_reg = (devreg_t *) dev_reg_addr;                                                              /* Device register del device che ha generato l'interrupt */

    /* Gli interrupt dei terminali vanno distinti dagli interrupt degli altri device */
    if (line != TERMINT)
        acknowledge(device_interrupting, line, (devreg_t *) dev_reg_addr, GENERAL_INT);
    else
        if (dev_reg->term.transm_status != READY)
            acknowledge(device_interrupting, line,(devreg_t *) dev_reg_addr, TERMTRSM_INT);
        else
            acknowledge(device_interrupting, line,(devreg_t *) dev_reg_addr, TERMRECV_INT);
}

void acknowledge(int device_interrupting, int line, devreg_t *dev_register, int type){
    int device_index = (line - 3) * 8 + device_interrupting + 1;                                /* Indice del semaforo su cui fare l'operazione di verhogen */


    if (line == TERMINT && type == TERMRECV_INT)
        device_index += DEVPERINT; 

    pcb_PTR to_unblock_proc = headBlocked(&(sem[device_index]));                            /* Processo da sbloccare, che è stato di wait */

    if (to_unblock_proc != NULL){
        switch (type){
            case GENERAL_INT:
                to_unblock_proc->p_s.reg_v0 = dev_register->dtp.status;
                dev_register->dtp.command = ACK;
                break;
            case TERMTRSM_INT:
                to_unblock_proc->p_s.reg_v0 = dev_register->term.transm_status;
                dev_register->term.transm_command = ACK;
                break;
            case TERMRECV_INT:
                to_unblock_proc->p_s.reg_v0 = dev_register->term.recv_status;
                dev_register->term.recv_command = ACK;
                break;
        }
        pcb_PTR unblocked = verhogen(&sem[device_index]);
        if (unblocked != NULL)
            soft_counter--; 
    }
    if (current_p == NULL) scheduler(); 
    else{
        current_p->p_time += exception_time - start_usage_cpu;
        STCK(start_usage_cpu); 
        copy_state(&(current_p->p_s),exception_state); 
        LDST(&(current_p->p_s)); 
    }
}

int get_dev_interrupting(memaddr *bitmap_word_addr){
    int device_interrupting = 0; 
    while(device_interrupting < DEVPERINT){
        if ((*bitmap_word_addr) & (1 << device_interrupting))
            return device_interrupting; 
        device_interrupting++; 
    }
    /* Non dovrebbe mai arrivare qui */
    return -1; 
}