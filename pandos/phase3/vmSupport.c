#include "../h/vmSupport.h"

// Swap pool mutex
int swap_pool_semaphore; 
// Vettore di interi usato per tenere traccia di quale processo possiede il mutex sulla swap pool
int swap_pool_holding[UPROCMAX]; 
// Swap pool
swap_t swap_pool[POOLSIZE]; 

extern pcb_PTR current_p; 
extern int flash_sem[UPROCMAX];

void initSwapStructs(){
	swap_pool_semaphore = 1;
	for (int i = 0; i < POOLSIZE; i++)
		swap_pool[i].sw_asid = NOPROC;
	for (int i = 0; i < UPROCMAX; i++)
		swap_pool_holding[i] = 0;
}

void pager(){
	// Recupero della struttura di supporto del processo corrente
	support_t *curr_support = (support_t *) SYSCALL(GETSUPPORTPTR, 0, 0, 0); 
	// Estrazione del Cause.ExcCode
	int cause = curr_support->sup_exceptState[PGFAULTEXCEPT].cause & GETEXECCODE; 
	cause >>= 2; 
	// TLB Modification, deve scattare una trap (poiche' non dovrebbe verificarsi)
	if (cause == 1)
		terminate(curr_support->sup_asid - 1);
	
	// Acquisizione della mutua esclusione sulla swap pool table
	SYSCALL(PASSEREN, (memaddr) &swap_pool_semaphore, 0, 0); 
	// Aggiornamento del vettore associato alla swap pool
	swap_pool_holding[curr_support->sup_asid - 1] = 1; 
	// Acquisizione del numero della pagina da caricare in memoria
	int page_missing = (curr_support->sup_exceptState[PGFAULTEXCEPT].entry_hi - KUSEG) >> VPNSHIFT; 

	if ((curr_support->sup_exceptState[PGFAULTEXCEPT].entry_hi >> VPNSHIFT) == 0xBFFFF)
		// Si tratta della pagina dello stack
		page_missing = MAXPAGES - 1;

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

		// Marcatura della page table entry come non valida
		swap_pool[frame_asid].sw_pte->pte_entryLO &= (~VALIDON); 
		// Aggiornamento del TLB, per garantire la coerenza dei dati 
		// TODO: aggiornare il TLB riscrivendo la entry usando TLBP e TLBWI
		TLBCLR(); 

		// Riabilitazione degli interrupt
		setSTATUS(getSTATUS() & IECON); 
		
		// Aggiornamento della memoria "secondaria" i.e. flash device associato al processo copiando il contenuto di in RAM del victim frame
		flash_device_operation(victim_frame,FLASHWRITE, curr_support); 	
	}

	// Lettura della pagina da caricare e scrittura in RAM nel victim frame
	flash_device_operation(victim_frame, FLASHREAD, curr_support); 
	
	// Disabilitazione degli interrupt
	setSTATUS(getSTATUS() & DISABLEINTS);

	// Aggiornamento della tabella della swap pool ai nuovi dati che occupano il frame 
	swap_pool[victim_frame].sw_asid = curr_support->sup_asid - 1; 
	swap_pool[victim_frame].sw_pageNo = page_missing; 
	swap_pool[victim_frame].sw_pte = &(curr_support->sup_privatePgTbl[page_missing]);

	// Aggiornamento della tabella delle pagine, ora la pagina si trova in memoria (bit V a 1)
	curr_support->sup_privatePgTbl[page_missing].pte_entryLO = victim_frame | VALIDON | DIRTYON; 

	// Aggiornamento del TLB, per garantire la coerenza dei dati 
	// TODO: aggiornare il TLB riscrivendo la entry usando TLBP e TLBWI
	TLBCLR();

	// Riabilitazione degli interrupt
	setSTATUS(getSTATUS() & IECON);
	
	// Aggiornamento del vettore associato alla swap pool
	swap_pool_holding[curr_support->sup_asid - 1] = 0; 
	
	// Rilascio della mutua esclusione sulla swap pool table
	SYSCALL(VERHOGEN, (memaddr) &swap_pool_semaphore, 0, 0); 

	// Ritorno del controllo al processo corrente perchè la pagina è stata caricata in memoria
	LDST(&(curr_support->sup_exceptState[PGFAULTEXCEPT])); 
}

// Algoritmo di rimpiazzamento FIFO
int replacement_algorithm(){
	// Variabile che contiene l'indice della prossima pagina vittima
	static int next_frame = 0; 
	int victim_frame = next_frame; 
	next_frame = (next_frame + 1) % POOLSIZE; 
	return victim_frame; 
}

void flash_device_operation(int frame, int operation, support_t *curr_support){
	// Ottenimento del frame asid coinvolto nell'operazione dal/sul flash device
	int asid = operation == FLASHWRITE ? swap_pool[frame].sw_asid : curr_support->sup_asid - 1;

	// Ricavo l'indirizzo del device register associato al flash device dell'asid passato come parametro
	memaddr dev_reg_addr = (memaddr) (DEVREGSTRT_ADDR + ((FLASHINT - 3) * 0x80) + (asid * 0x10));    /* Indirizzo del flash device */
    devreg_t *dev_reg = (devreg_t *) dev_reg_addr;
	
	// Acquisizione del mutex sul flash device (per la manipolazione dei device register)
	SYSCALL(PASSEREN, (memaddr) &flash_sem[curr_support->sup_asid - 1], 0, 0);

	// Operazione di scrittura sul / lettura dal flash device, seguendo il formato descritto in 5.4 pops
	dev_reg->dtp.data0 = (memaddr) (KUSEG + (frame * PAGESIZE));
	dev_reg->dtp.command = (swap_pool[frame].sw_pageNo - KUSEG) >> VPNSHIFT | operation; 

	// Scrittura sul / lettura dal flash device asid-esimo
	int flash_status = SYSCALL(DOIO, (memaddr) &(dev_reg->dtp.command), operation, 0); 
	
	// Rilascio del mutex del flash device
	SYSCALL(VERHOGEN, (memaddr) &flash_sem[curr_support->sup_asid - 1], 0, 0);

	// Se si è verificato un errore, scatta una trap
	if (flash_status != READY)
		terminate(curr_support->sup_asid - 1);
	
}