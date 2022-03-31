#ifndef INTERRUPTS
#define INTERRUPTS

#include "./types.h"
#include "./pandos_const.h"

void interrupt_handler(state_t* exception_state); 

#endif