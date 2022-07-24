4 file: init.c scheduler.c syscalls.c exceptions.c

init.c:

	1. Dichiarare (e inizializzare) le variabili globali di liv. 3:
		-	p_counter = 0 													/* processi iniziati ma non terminati */
		-	soft_counter = 0												/* processi che sono in stato `waiting` */
		-	ready_q = mkEmptyProcQ() 										/* puntatore alla tail della coda dei pcb in stato ready */
		-	current_p = NULL 												/* puntatore al pcb che sta nel `running` state */
		-	for (int i=0; i < MAX_DEVICES = 5 * 8 + 1; i++) sem[i] = 0; 	/* inizializza i semafori associati ai dispositivi */

	2. Popolare il passupvector (struct che si trova in types.h)
		passupvector_t *passupvector = (passupvector_t*) PASSUPVECTOR; 
		passupvector->tlb_refill_handler = (memaddr) uTLB_RefillHandler; 
		passupvector->tlb_refill_stackPtr = KERNELSTACK
		passupvector->exception_handler = (memaddr) exceptionHandler; 
		passupvector->exception_stackPtr = KERNELSTACK
		dove KERNELSTACK si trova in pandos_const.h, uTLB_RefillHandler in p2test.c, exceptionHandler in exceptions.c. 

	3. Inizializzare le strutture dati di livello 2 con initPcbs(), initSemd()

	4. Inizializzare le variabili del punto 1
	
	5. Istanziare un processo (allocPcb()) a priorità bassa (campo p_prio di pcb_t) e posizionarlo nella ready_q definita nel punto 1 (insertProcQ()).
	   Inoltre, inizializzare lo stato del processore (struct che si trova in types.h) associato al pcb (process->p_s), con:
		- Interrupt abilitati
		- Local Timer del processore abilitato
		- Modalità kernel on
			(process->p_s) = IEPON | TEBITON | ? 
		- SP settato a RAMTOP
			RAMTOP((process->p_s).reg_sp);
			//gpr[26] is SP, page 6 pandOS guide. 
		- PC settato all'address della funzione test di p2test.c
			(process->p_s).pc_epc = (memaddr) test; 
			I rimanenti campi del pcb_t da settare a NULL o 0.
			
	6. Chiamare lo scheduler.

scheduler.c: 

	Scheduler che utilizza un algoritmo di tipo round-robin con valore del time slice di 5 millisecondi per i processi a bassa priorità.
	Per i processi ad alta priorità lo scheduler si comporta come non-preemptive first come first served.

	E' necessario utilizzare il PLT (processor local timer) per generare interrupt per i processi a bassa priorità. 

		- Se la coda dei processi ready ad alta priorità è non vuota: 
			1. Rimuovi la testa di tale coda e assegna il puntatore a tale pcb a current_p
			2. Carica lo stato del processore del pcb (p_s) sul processore tramite una chiamata LDST.
		- Altrimenti: 
			1. Rimuovi la testa della coda dei processi a bassa priorità a assegna tale pcb a current_p
			2. Carica 5 millisecondi sul PLT
			3. Carica lo stato del processore del pcb (p_s) sul processore tramite una chiamata LDST.
		- Se entrambe le code sono vuote:
			1. p_counter == 0 => HALT()
			2. p_counter > 0 && soft_counter > 0 => WAIT(). Prima di chiamare WAIT(), lo scheduler deve settare il registro di stato del
			   processore per abilitare gli interrupt e disabilitare il PLT (o caricarlo con un valore molto grande perchè il primo interrupt
			   che avviene dopo la WAIT() non deve essere del PLT). 
			3. p_counter > 0 && soft_counter == 0 => PANIC(). Deadlock.

exceptions.c: 
	Questo modulo implementa i TLB, Program Trap a i Syscall exception handlers.
	Questo modulo conterrà lo scheletro del TLB-Refill event handler che si trova in p2test.c

	Se il passupvector è stato inizializzato correttamente, exceptionHandler() verrà chiamato ogni volta che occorrerà un'eccezione non di tipo TLB-Refill.
	Inoltre lo stato del processore al momento dell'eccezione verrà salvato (per il CPU0) all'inizio del BIOSDATAPAGE. Quindi si dovrà dichiarare un punatore
	a status_t e inizializzarlo nel seguente modo: 

		status_t *cpu_status = BIOSDATAPAGE; 
		int handler = cpu_status->cause & 0b00000000000000000000000001111100

		switch (handler) {
			case IOINTERRUPTS:
				interruptHandler(); 
				break; 
			case 1:
			case TLBINVLDL:
			case TLBINVLDS:
				tlbHandler(); 
				break; 
			case SYSEXCEPTION: 
				syscallHandler(); 
				break; 
			default: 
				trapHandler(); 
				break; 
		}

	syscallHandler(): 
		Per convenzione, il processo in esecuzione piazza i valori (parametri) nei registri da a0 a a3 (gpr[3]-gpr[6]) prima di eseguire l'istruzione SYSCALL.
		Syscall performerà la syscall a seconda del valore nel registro a0 che deve essere negativo, altrimenti si tratta di PassUpOrDie (syscall trap):

			Create_Process(NSYS1): [ a0 = -1; typeof(a1) = (state_t *); a2 = p_prio; typeof(a3) = (support_t *);]

			In questo caso il processo che ha usato la syscall deve essere il genitore del processo creato.
			Nel registro a1 c'è un puntatore allo stato del processore prima della chiamata della syscall (state_t *).
			Questo stato deve essere lo stato iniziale del nuoovo processo. Il processo che ha chiamato NSYS1 continua ad esistere e ad eseguire.
			Se non è possibile creare il nuovo processo per mancanza di risorse, -1 è ritornato in v0 (gpr[1]) del chiamante. Altrimenti è ritornato il pid in v0. 

			void createProcess(void *a1, state_t *a2, support_t *a3) {
				pcb_PTR newProcess = allocPcb(); 
				if (newProcess == NULL) *a1 = -1; 
				else {
					...
					...
					*a1 = newProcess->p_pid; 
				}
			}

interrupts.c:

	Introduzione:
		Questo file implementa i device/time interrupt exception handler.
		Tali handlers verranno convertite in operazioni di verhogen() sull'opportuno semaforo verhogen(sem[device_index]).

		interrupt_handler (state_t *exception_state):
			Questa funzione deve andare a gestire, in base all'ordine di priorità, gli interrupt in attesa.

			Per capire quale interrupt si è verificato, c'è il field InterruptsPending(IP) del registro cause.
			Si tratta di un field di 8 bit che indica quali sono le linee di interrupt in cui c'è un interrupt in attesa.
			Se un interrupt è in attesa sulla linea i, allora il Cause.IP[i] sarà 1. 

			Per estrarre tale campo: 
			int ip = exception_state->cause & IMON

			esempio: 
				- C'è solo un interrupt ed è in attesa sulla linea 3 (disk interrupt) => il campo IP del registro cause sarà: 0000 1000 (il bit 3 è acceso).
				- Poichè il registro cause è un registro a 32 bit, se tutti i bit sono a 0 tranne il bit del campo IP che indica che c'è un interrupt sulla linea 3 in attesa,
				il valore del registro (in decimale) sarà: 2048 (proprio per questo motivo DISKINTERRUPT = 0x00000800 che in decimale è 2048)

		Il nostro kernel deve gestire solo le linee di interrupt dalla 1 alla 7. Quindi:

		if (ip == LINE1) {

		} else if (ip == LINE2) {

		} ....

	About LINES:
		Ogni linea deve essere gestita da un handler specifico (e.g. line_i_handler()); 
		Per le linee di interrupt da 3 a 7 è necessario identificare il device sulla linea che ha provocato l'interrupt.
		Questo si fa attraverso la interrupting device bitmap, un'area di memoria di 4 word che inizia all'indirizzo 0x10000040.
		Ogni word di tale area è riservata per linea di interrupt per indicare quale device ha provocato l'interrupt.

	About BITMAP:
		Come funziona la interrupting device bitmap? 
			Quando il bit i della word j è posto a 1 allora il device i della linea j+3 ha un interrupt in attesa su tale linea.
			La bitmap è gestita automaticamente dall'hardware => noi non ci dobbiamo preoccupare di accendere bit su tale area di memoria.

	What we need 2 do:
		- Capire attraverso la bitmap quale device ha generato l'interrupt:
		
			1. Per fare questo dobbiamo prima posizionarci all'interno della word corrispondente nella bitmap in base alla linea del field ip: 

				#define ADDRBITMAPSTART 0x10000040					/* Indirizzo di inizio della Interrupting device BITMAP */
				int line = ip >> 8; 								/* Per migliorare la manipolazione della linea */
				memaddr bitmap_word_address = (memaddr) (ADDRBITMAPSTART) + (line - 3) * 0x04; 


			2. Dopodichè bisogna capire quale device ha generato l'interrupt.
			Questo si può fare con un ciclo del tipo: 
			
				int device_interrupting = 0; 								/* Intero che rappresenta il device che ha generato l'interrupt */
				while (device_interrupting < DEVPERINT) {					/* Maschera di bit */
					if (bitmap_word_address & (1 << device_interrupting))
						break; 
					device_interrupting++; 
				}
		
			3. Fare l'acknowledge dell'interrupt (equivalente a spegnere il bit sulla bitmap).
			   Questo si può fare scrivendo il valore ACK sul field COMMAND del device register.

				3.1. Per fare questo dobbiamo prima trovare l'indirizzo del device register.
					 Tutti i device register di uMPS3 si trovano in un'area di memoria che inizia all'indirizzo 0x10000054.
		
				A seguire è riportato il manuale di uMPS3 (sezione 5.1, device registers): 
					Given an interrupt line (IntLineNo) and a device number (DevNo) one can
					compute the starting address of the device’s device register:
						devAddrBase = 0x1000.0054 + ((IntlineNo - 3) * 0x80) + (DevNo * 0x10)

				Quindi per trovare l'indirizzo del device register, una volta nota la linea e il numero del device che ha generato l'interrupt,
				basta seguire le indicazioni. Nel nostro caso:
					/* dev_addr_reg = indirizzo del device register */
					memaddr dev_addr_reg = 0x1000.0054 + ((line - 3) * 0x80) + (device_interrupting * 0x10); 

				3.1.2. Prima di scrivere sul campo command del register device, bisogna salvare lo status code del device register sul registro
					reg_v0del processo che passa in stato ready. Poichè gli interrupt occorrono quando è stata completata una richiesta di I/O
					(o quando il PLT si azzera o quando l'interval timer si azzera, ma non ci interessano per il momento).
					Allora significa che in precedenza un processo aveva richiesto una operazione di I/O tramite la syscall do_io(). 
					Quindi, facendo l'operazione di I/O si era bloccato sul semaforo del dispositivo tramite insertBlocked(&sem[device],processo); 
			
					Quindi, per ritrovare il pcb del processo che adesso può passare dallo stato blocked allo stato ready, basta guardare la coda dei processi
					bloccati associata al semd con chiave sem[device].

					pcb_PTR processo = headBlocked(&sem[index]); 
					processo->reg_v0 = dev_register->type; 

						- Dove "index" è l'indice del semaforo del dispositivo che ha generato l'interrupt. 
						index = (line-3) * 8 + device_interrupting;  
						- Dove type può assumere 3 valori in base a quale device ha generato l'interrupt: 
							- se è un device di tipo terminale (recv) => type = term.recv_status; 
							- se è un device di tipo terminale (trasm) => type = term.trasm_status; 
							- se è un device generale (non terminale) => type = dtp.status; 

				3.2. Ora bisogna scrivere un ACK sul campo command del register device.
					 devreg_t *reg = (devreg_t *) dev_addr_reg; 
					 reg->type = ACK; 

					 - Dove type può assumere 3 valori in base a quale device ha generato l'interrupt: 
						- se è un device di tipo terminale (recv) => type = term.recv_command; 
						- se è un device di tipo terminale (trasm) => type = term.trasm_command; 
						- se è un device generale (non terminale) => type = dtp.command; 

			Quindi riassumendo, per i device non di tipo timer avremo degli handler del tipo: 

				void interrupt_handler(cput_t *exception_state) {
					int ip = exception_state->cause & IMON; 

					if (ip == LINE1) {
						line_1_handler(ip); 
					}....
				}

				void line_1_handler(int ip) {
					#define ADDRBITMAPSTART 0x10000040
					int line = ip >> 8; 							/* Per migliorare la manipolazione della linea */
					memaddr bitmap_word_address = (memaddr) (ADDRBITMAPSTART) + (line - 3) * 0x04; 

					int device_interrupting = 0;					/* Intero che rappresenta il device che ha generato l'interrupt, inizializzato a 0 */ 
					while (device_interrupting < DEVPERINT) {		/* Maschera di bit */
						if (bitmap_word_address & (1 << device_interrupting)) break; 
						device_interrupting++; 
					}

					/* Indirizzo del device register */
					memaddr dev_addr_reg = (memaddr) 0x1000.0054 + ((line - 3) * 0x80) + (device_interrupting * 0x10); 
					
					#define GENERAL_INTERRUPT 0
					int type = GENERAL_INTERRUPT; 

					acknowledge(line, device_interrupting, dev_addr_reg, type); 
				}

				void acknowledge(int line, int device_interrupting, devreg_t *dev_reg, int type) {
					int device_index = (line - 3) * 8 + device_interrupting; 

					pcb_PTR to_unblock_proc = headBlocked(&(sem[device_index]));  

					if (to_unblock_proc != NULL) {
						switch(type) {
							case GENERAL_INTERRUPT: 
								to_unblock_proc->p_s.reg_v0 = dev_reg->dtp.status; 
								dev_reg->dtp.command = ACK; 
								break; 
							case TERM_RECV: 
								to_unblock_proc->p_s.reg_v0 = dev_reg->dtp.status; 
								dev_reg->term.recv_command = ACK; 
								break; 
							case TERM_TRSM:
								to_unblock_proc->p_s.reg_v0 = dev_reg->dtp.status; 
								dev_reg->term.trasm_command = ACK; 
								break; 
						}
						verhogen(&(sem[device_index])); 
					}
				}