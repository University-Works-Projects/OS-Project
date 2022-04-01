#ifndef INTERRUPTS
#define INTERRUPTS

#include "./types.h"
#include "./pandos_const.h"

void interrupt_handler(state_t* exception_state); 

/* Interrupt Handlers */

/* I.H for disk devices */
void disk_interrupt_handler(int ip); 

/* I.H for flash devices */
void flash_interrupt_handler(int ip); 

/* I.H for network adapters*/
void network_interrupt_handler(int ip); 

/* I.H for printer devices */
void printer_interrupt_handler(int ip); 

/* I.H for terminal devices */
void terminal_interrupt_handler(int ip); 

/* 
    Acknowledge function for I/O interrupts. 
    This function performs an ack on the dev_register and also a v on the appropriate semaphores.
*/
void acknowledge(int device_interrupting, int line, devreg_t *dev_register, int type); 

#endif