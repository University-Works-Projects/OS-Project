#ifndef SYSSUPPORT 
#define SYSSUPPORT

#include "../testers/h/tconst.h"    /* To use the constant values in the switch case in the .c file */
#include "types.h"
#include "pandos_const.h"
#include "pandos_types.h"
#include "libumps.h"
#include "interrupts.h"

#define PRINTCHR 2
#define TERMSTATMASK 0xFF
#define RECVD    5

void general_exception_handler();

void get_tod (state_t *exception_state);

void terminate (int asid);

void write_to_printer (state_t *exception_state, int asid);

void write_to_terminal (state_t *exception_state, int asid);

void read_from_terminal ();


#endif