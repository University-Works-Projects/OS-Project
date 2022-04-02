#include "../h/interrupts.h"

void interrupt_handler(state_t* exception_state){
    /* Estrazione del campo IP dal registro CAUSE */
    int ip = exception_state->cause & IMON;

    /* La priorità delle chiamate è implementata in base all'ordine di attivazione dei seguenti if */
    if (ip && LOCALTIMERINT) {
        //TODO: plt_handler(); 
    } else if (ip && TIMERINTERRUPT) {
        //TODO: interval_timer_handler(); 
    } else if (ip && DISKINTERRUPT) {
        non_timer_interrupt(DISKINT);     
    } else if (ip && FLASHINTERRUPT) { 
        non_timer_interrupt(FLASHINT);        
    } else if (ip && NETINTERRUPT){
        non_timer_interrupt(NETWINT); 
    } else if (ip && PRINTINTERRUPT) { 
        non_timer_interrupt(PRNTINT); 
    } else if (ip && TERMINTERRUPT) { 
        non_timer_interrupt(TERMINT); 
    }
}

void non_timer_interrupt(int line){
    memaddr bitmap_word_addr = (memaddr) (BITMAPSTRT_ADDR) + (line - 3) * 0x04;                                 /* Indirizzo d'inizio della word della interrupting bitmap rispettiva alla linea passata come parametro */
    int device_interrupting = get_dev_interrupting(bitmap_word_addr);                                           /* Numero del device che ha provocato l'eccezione */
    memaddr dev_reg_addr = (memaddr) (DEVREGSTRT_ADDR + ((line - 3) * 0x80) + (device_interrupting * 0x10));    /* Inidirizzo del device register del device che ha provocato l'eccezione */
    devreg_t *dev_reg = (devreg_t *) dev_reg_addr;                                                              /* Device register del device che ha generato l'interrupt */

    /* Gli interrupt dei terminali vanno distinti dagli interrupt degli altri device */
    if (line != TERMINT)
        acknowledge(device_interrupting, line, dev_reg_addr, GENERAL_INT);
    else
        if (dev_reg->term.transm_status != READY && dev_reg->term.transm_status != BUSY)
            acknowledge(device_interrupting, line, dev_reg_addr, TERMTRSM_INT);
        else
            acknowledge(device_interrupting, line, dev_reg_addr, TERMRECV_INT);
}

void acknowledge(int device_interrupting, int line, devreg_t *dev_register, int type){
    int device_index = (line - 3) * 8 + device_interrupting + 1;                                /* Indice del semaforo su cui fare l'operazione di verhogen */
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
        verhogen(sem[device_index]);
    }
}

int get_dev_interrupting(memaddr bitmap_word_addr){
    int device_interrupting = 0; 
    while(device_interrupting < DEVPERINT){
        if (bitmap_word_addr & (1 << device_interrupting))
            return device_interrupting; 
        device_interrupting++; 
    }
    /* Non dovrebbe mai arrivare qui */
    return -1; 
}