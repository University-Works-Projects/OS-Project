#include "../h/initProc.h"

// Semafori dei device
int printer_sem[UPROCMAX],
    tread_sem[UPROCMAX],
    twrite_sem[UPROCMAX]; 