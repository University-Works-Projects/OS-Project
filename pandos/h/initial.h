#ifndef INITIAL
#define INITIAL

#include "pcb.h"
#include "asl.h"
#include "exceptions.h"
#include "interrupts.h"
#include "scheduler.h"
#include "types.h"

#define DEVICE_INITIAL 49

/* Array dei semafori dei dispositivi */
int sem[DEVICE_INITIAL];
/* Intero che rappresenta rispettivamente il numero di processi "vivi" e il numero di processi bloccati per I/O */
int p_count, soft_counter;
/* Coda dei processi in stato ready ad alta (hq) e bassa (lq) priorità */
pcb_t *ready_hq, *ready_lq, *current_p;

passupvector_t *passupvector; 

#endif