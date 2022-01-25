#include "pcb.h"

HIDDEN LIST_HEAD(pcbFree_h);            /* Lista dei PCB liberi. */
HIDDEN pcb_t pcbFree_table[MAXPROC];    /* Tabella contenente tutti i PCB. */

void initPcbs() {
    for (int i = 0; i < MAXPROC; i++) list_add_tail(&(pcbFree_table[i].p_list), &pcbFree_h);    /* Scorre pcbFree_table e inserisce i p_list dei suoi elementi in pcbFree_h */
}

void freePcb(pcb_t* p) {

}

pcb_t* allocPcb() {

}

void mkEmptyProcQ(struct list_head* head) {

}

int emptyProcQ(struct list_head* head) {

}

void insertProcQ(struct list_head* head, pcb_t* p) {

}

pcb_t* headProcQ(struct list_head* head) {

}

pcb_t* removeProcQ(struct list_head* head) {

}

pcb_t* outProcQ(struct list_head* head, pcb_t* p) {

}

int emptyChild(pcb_t* p) {

}

void insertChild(pcb_t* prnt, pcb_t* p) {

}

pcb_t* removeChild(pcb_t* p) {

}

pcb_t* outChild(pcb_t* p) {

}
