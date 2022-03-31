#include "../h/interrupts.h"

void interrupt_handler(state_t* exception_state){
    /**
     * Il nucleo deve gestire le linee di interrupt da 1 a 7.
     * 
     * Azioni che il nucleo deve svolgere:
     *  1. Identificare la sorgente dell’interrupt
     *      - Linea: registro Cause.IP
     *      - Device sulla linea (>3): Interrupting Device Bit Map
     *  2. Acknowledgment dell’interrupt
     *      - Scrivere un comando di ack (linea >3) o un nuovo comando
     *        nel registro del device.
     * 
     * Interrupt con numero di linea più bassa hanno priorità più alta, e
     * dovrebbero essere gestiti per primi.
     * 
     * 
     * The interrupt exception handler’s first step is to determine which
     * device or timer with an outstanding interrupt is the highest priority.
     * Depending on the device, the interrupt exception handler will perform a
     * number of tasks.
     */
    
    /* 
        ip =     xxxxxxxx
                 1 0000000 >> 8
    */
    int ip = exception_state->cause & IMON;

    /**
     * La priorità delle chiamate è implementata in base all'ordine di attivazione
     * dei seguenti if.
     */
    if (ip && LOCALTIMERINT) {
        
    } else if (ip && TIMERINTERRUPT) {

    } else if (ip && DISKINTERRUPT) {
        
    } else if (ip && FLASHINTERRUPT) { 
        
    } else if (ip && PRINTINTERRUPT) { 
        
    } else if (ip && TERMINTERRUPT) { 
        
    }
}