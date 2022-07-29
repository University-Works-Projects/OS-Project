#include "../h/initProc.h"

// Semafori dei device per proteggere l'accesso ai device register
int printer_sem[UPROCMAX],
    tread_sem[UPROCMAX],
    twrite_sem[UPROCMAX]; 
    flash_sem[UPROCMAX];

void test(){
    // Inizializzazione strutture dati della memoria virtuale
    initSwapStructs();
    // Inizializzazione dei semafori dei device
    for (int i = 0; i < UPROCMAX; i++){
        printer_sem[i] = 1;
        tread_sem[i] = 1;
        twrite_sem[i] = 1;
        flash_sem[i] = 1;
    }
    for (int i = 0; i < UPROCMAX; i++){
        state_t initial_state;
        support_t sup_struct; 
        SYSCALL(CREATEPROCESS, &initial_state, &sup_struct, 0);
    }
}