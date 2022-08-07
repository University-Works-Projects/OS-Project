#include "../h/initProc.h"

// Stati e strutture di supporto dei processi utente
HIDDEN support_t uproc_support[UPROCMAX];
HIDDEN state_t uproc_state[UPROCMAX];

// Semafori dei device per proteggere l'accesso ai device register
int printer_sem[UPROCMAX],
    tread_sem[UPROCMAX],
    twrite_sem[UPROCMAX],
    flash_sem[UPROCMAX];

// Semaforo per non terminare test prima che tutti gli U-proc siano terminati
int block_sem;

// Semaforo per aspettare la terminazione dei processi creati da test. 
int master_semaphore; 

extern void general_exception_handler();
extern void pager();

void test(){
    //Inizializzazione del master_semaphore
    master_semaphore = 0; 
    // Inizializzazione strutture dati della memoria virtuale
    initSwapStructs();
    // Inizializzazione dei semafori dei device
    for (int i = 0; i < UPROCMAX; i++){
        printer_sem[i] = 1;
        tread_sem[i] = 1;
        twrite_sem[i] = 1;
        flash_sem[i] = 1;
    }

    // Ciclo di inizializzazione dei processi utente
    for (int i = 0; i < UPROCMAX; i++){
        // Il PC e il registro t9 devono essere inizializzati all'inizio della sezione .text 
        uproc_state[i].pc_epc = (uproc_state[i].reg_t9 = UPROCSTARTADDR);

        
        // Inizializzazione dello stack pointer
        uproc_state[i].reg_sp = USERSTACKTOP;

        /* 
            Quando si verifica un interrupt della linea i, se gli interrupt sono abilitati,
            il processore accetta l'interrupt solo se il bit Status.IM[i] e' acceso.
            IMON accende tutti i Status.IM.
            USERPON disattiva la kernel mode, settando il bit KUp a 1. E' necessario settare KUp e non KUc
            perche' al momento in cui si verifica un'eccezione il bit KUc viene mosso in KUp, e KUc viene posto a 0.
            Quando viene effettutata una operazione di LDST, avviene un'operazione complementare,
            per cui per disattivare la kernel mode, basta porre KUp a 1.
            Discorso analogo per IEp/IEc, la cui funzione e' quella di attivare / disattivare gli interrupt.
        */
        uproc_state[i].status = TEBITON | IMON | USERPON | IEPON;
        uproc_state[i].entry_hi = (i + 1) << ASIDSHIFT;

        // Ad ogni processo utente deve essere assegnato un asid di valore strettamente positivo e unico
        uproc_support[i].sup_asid = i + 1;

        // Inizializzazione dei campi PC usati dal nucleo per passare la gestione al livello di supporto.
        uproc_support[i].sup_exceptContext[GENERALEXCEPT].pc = (memaddr) general_exception_handler;
        uproc_support[i].sup_exceptContext[PGFAULTEXCEPT].pc = (memaddr) pager;
        // Modalita' kernel accesa (bit KUp a 0), interrupt abilitati e PLT abilitato
        uproc_support[i].sup_exceptContext[GENERALEXCEPT].status = IMON | IEPON | TEBITON;
        uproc_support[i].sup_exceptContext[PGFAULTEXCEPT].status = IMON | IEPON | TEBITON;

        // Ricavo del RAMTOP
        memaddr ram_top; 
        RAMTOP(ram_top);

        /* 
            Inizializzazione degli stack utilizzati dal pager e dal general exception handler. 
            Come raccomandato dal manuale di pandosplus, e' utilizzata la RAM strettamente al di sotto
            del RAMTOP, evitando l'ultimo frame della RAM che e' riservato allo stack del test
        */
        uproc_support[i].sup_exceptContext[PGFAULTEXCEPT].stackPtr = (memaddr) (ram_top - (i + 1) * PAGESIZE * 2);
        uproc_support[i].sup_exceptContext[GENERALEXCEPT].stackPtr = (memaddr) (ram_top - (i + 1) * PAGESIZE * 2 + PAGESIZE);
        // Ciclo di inizializzazione della page table degli u-proc
        for (int j = 0; j < MAXPAGES; j++){
            if (j == MAXPAGES - 1)
                // L'ultima pagina e' la pagina di stack, il VPN deve essere settato a 0xBFFFF
                uproc_support[i].sup_privatePgTbl[j].pte_entryHI = 0xBFFFF << VPNSHIFT;
            else
                // Inizializzazione della VPN, campo della entryHI che comincia dal bit VPNSHIFT
                uproc_support[i].sup_privatePgTbl[j].pte_entryHI = KUSEG + (j << VPNSHIFT);
            // Inizializzazione dell'asid, campo della entryHI che comincia dal bit ASIDSHIFT
            uproc_support[i].sup_privatePgTbl[j].pte_entryHI |= (uproc_support[i].sup_asid) << ASIDSHIFT;
            // La pagina non si trova in memoria quindi basta porre i bit V a 0, D a 1 (protezione della memoria disattivata)
            uproc_support[i].sup_privatePgTbl[j].pte_entryLO = DIRTYON;
        }

        // NSYS1
        if (SYSCALL(CREATEPROCESS, (memaddr) &uproc_state[i], PROCESS_PRIO_LOW, (memaddr) &uproc_support[i]) < 0){
            // Termina il processo di test, non e' stato possibile creare un processo
            SYSCALL(TERMINATE, 0, 0, 0);
        }
    }
    for (int i = 0; i < UPROCMAX; i++)
        SYSCALL(PASSEREN, (memaddr) &master_semaphore, 0, 0);
    SYSCALL(TERMPROCESS, 0, 0, 0);
}