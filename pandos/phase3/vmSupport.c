#include "../h/vmSupport.h"

/* Swap pool mutex */
int swap_pool_semaphore = 1; 
/* Swap pool */
swap_t swap_pool[POOLSIZE]; 

extern pcb_PTR current_p; 

void pager(){
	// Recupero della struttura di supporto del processo corrente
	support_t *curr_support = (support_t *) SYSCALL(GETSUPPORTPTR, 0, 0, 0); 
	// Estrazione del Cause.ExcCode
	int cause = curr_support->sup_exceptState[0].cause & GETEXECCODE; 
	cause >>= 2; 

	if (cause == 1){
		// TLB-Modification exception, si gestisce come una program trap
	}
	
	// Acquisizione della mutua esclusione sulla swap pool table
	SYSCALL(PASSEREN, &swap_pool_semaphore, 0, 0); 
	
	// Acquisizione del numero della pagina da caricare in memoria
	int page_missing = curr_support->sup_exceptState[0].entry_hi >> VPNSHIFT; 

	// Rilascio della mutua esclusione sulla swap pool table
	SYSCALL(VERHOGEN, &swap_pool_semaphore, 0, 0); 

}
