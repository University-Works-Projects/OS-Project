#ifndef SYSSUPPORT 
#define SYSSUPPORT

#include "../testers/h/tconst.h"    /* To use the constant values in the switch case in the .c file */
#include "types.h"
#include "pandos_const.h"
#include "pandos_types.h"

void general_exception_handler();

void get_tod ();

void terminate ();

void write_to_printer ();

void write_to_terminal ();

void read_from_terminal ();


#endif