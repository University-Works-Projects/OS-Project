#include "../h/interrupts.h"

void interrupt_handler(state_t* exception_state){
    /* Estrazione del campo IP dal registro CAUSE */
    int ip = exception_state->cause & IMON;

    /* La priorità delle chiamate è implementata in base all'ordine di attivazione dei seguenti if */
    if (ip && LOCALTIMERINT) {
    
    } else if (ip && TIMERINTERRUPT) {

    } else if (ip && DISKINTERRUPT) {
        disk_interrupt_handler(ip);     
    } else if (ip && FLASHINTERRUPT) { 
        flash_interrupt_handler(ip);        
    } else if (ip && 0x00002000){
        network_interrupt_handler(ip); 
    } else if (ip && PRINTINTERRUPT) { 
        printer_interrupt_handler(ip); 
    } else if (ip && TERMINTERRUPT) { 
        terminal_interrupt_handler(ip); 
    }
}

void disk_interrupt_handler(int ip){

}

void flash_interrupt_handler(int ip){

}

void network_interrupt_handler(int ip){

}

void printer_interrupt_handler(int ip){

}

void terminal_interrupt_handler(int ip){

}