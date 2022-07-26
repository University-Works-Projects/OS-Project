#ifndef SYSSUPPORT 
#define SYSSUPPORT

#include "../testers/h/tconst.h"    /* To use the constant values in the switch case in the .c file */
#include "pandos_const.h"
#include "pandos_types.h"

void generalException_hanlder();

/* NSYS11 */
void get_tod ();

/* NSYS12 */
void terminate ();

/* NSYS13 */
void write_to_printer ();

/* NSYS14 */
void write_to_terminal ();

/* NSYS15 */
void read_from_terminal ();


#endif