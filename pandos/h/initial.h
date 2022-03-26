#ifndef INITIAL
#define INITIAL

#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/exceptions.h"
#include "../h/interrupts.h"
#include "../h/scheduler.h"
#include "../h/types.h"

#define N_DEVICE 48

extern void test();
extern void uTLB_RefillHandler();
extern void exception_handler();

/* Array dei semafori dei dispositivi */
int sem[N_DEVICE];
/* Intero che rappresenta rispettivamente il numero di processi "vivi" e il numero di processi bloccati per I/O */
int p_count, soft_counter;
/* Coda dei processi in stato ready ad alta (hq) e bassa (lq) priorità */
pcb_t *ready_hq, *ready_lq, *current_p;

passupvector_t *passupvector;

#endif