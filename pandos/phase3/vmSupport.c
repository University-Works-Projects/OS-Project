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
	int page_missing = (curr_support->sup_exceptState[0].entry_hi - KUSEG) >> VPNSHIFT; 

	int victim_frame = -1; 
	// Ciclo per trovare un frame libero nella swap_pool
	while(++victim_frame < POOLSIZE)
		if (swap_pool[victim_frame].sw_asid == NOPROC)
			break; 
	
	// Non è stato trovato un frame libero, si deve chiamare l'algoritmo di rimpiazzamento
	if (victim_frame == POOLSIZE)
		victim_frame = replacement_algorithm(); 
	
	int frame_asid = swap_pool[victim_frame].sw_asid; 
	// Il frame "vittima" è occupato dalla pagina di un processo
	if (frame_asid != NOPROC){
		// Disabilitazione degli interrupt
		setSTATUS(getSTATUS() & DISABLEINTS); 

		// Riabilitazione degli interrupt
		setSTATUS(getSTATUS() & IECON); 
	}

	// Rilascio della mutua esclusione sulla swap pool table
	SYSCALL(VERHOGEN, &swap_pool_semaphore, 0, 0); 

}

// Algoritmo di rimpiazzamento FIFO
int replacement_algorithm(){
	// Variabile che contiene l'indice della prossima pagina vittima
	static int next_frame = 0; 
	int victim_frame = next_frame; 
	next_frame = (next_frame + 1) % POOLSIZE; 
	return victim_frame; 
}