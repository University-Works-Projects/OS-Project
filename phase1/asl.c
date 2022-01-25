#include "asl.h"

static semd_t semd_table[MAXPROC]; /* array di SEMD con dimensione massima di MAX_PROC */
static struct list_head semdFree_h; /* Lista dei SEMD liberi o inutilizzati */
static struct list_head semd_h; /* Lista dei semafori attivi */

int insertBlocked(int* semAdd, pcb_t* p) {

}

pcb_t* removeBlocked(int* semAdd) {

}

pcb_t* outBlocked(pcb_t* p) {

}

pcb_t* headBlocked(int* semAdd) {

}

void initASL() {
    if(&semdFree_h == NULL){//da testare
        /* Inizializziamo a lista vuota */
		INIT_LIST_HEAD(&semdFree_h);
		for (int i=0; i<MAXPROC; i++){
			semd_t e = semd_table[i];
			list_add_tail(&e, &semdFree_h);
		}
	}
}
