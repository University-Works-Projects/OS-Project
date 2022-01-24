#include "pcb.h"

HIDDEN pcb_t *pcbFree_h;                /* Lista dei pocessi liberi. */
HIDDEN pcb_t pcbFree_table[MAXPROC];    /* Tabella contenente tutti i preocessi. */

void initPcbs (void) {

}

void freePcb (pcb_t *p) {

}

pcb_t *allocPcb (void) {

}

pcb_t *mkEmptyProcQ (void) {

}

int emptyProcQ (pcb_t *tp) {

}

void insertProcQ (pcb_t **tp, pcb_t *p) {

}

pcb_t *headProcQ (pcb_t *tp) {

}

pcb_t *removeProcQ (pcb_t **tp) {

}

pcb_t *outProcQ (pcb_t **tp, pcb_t *p) {

}

int emptyChild (pcb_t *p) {

}

void insertChild (pcb_t *prnt, pcb_t *p) {

}

pcb_t *removeChild (pcb_t *p) {

}

pcb_t *outChild (pcb_t *p) {

}


