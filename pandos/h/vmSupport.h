#ifndef VMSUPPORT
#define VMSUPPORT  
#include <umps3/umps/libumps.h>
#include "pandos_const.h"
#include "pandos_types.h"
#include "interrupts.h"
#include "sysSupport.h"

#define POOLSTART 0x20020000

// Page fault exception handler
void pager(); 

// Algoritmo di rimpiazzamento 
int replacement_algorithm(); 

// Funzione che esegue una operazione in base al valore di operation sul flash device appropriato
void flash_device_operation(int frame, int operation, support_t *curr_support, int block_number);

// Funzione di inizializzazione della swap pool table, del semaforo associato e del vettore swap_pool_holding
void initSwapStructs();

// Funzione di aggiorna il TLB utilizzando IndexCP0 come indice
void refresh_TLB(pteEntry_t *updated_entry);

#endif
