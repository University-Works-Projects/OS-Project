#include "pcb.h"

HIDDEN LIST_HEAD(pcbFree_h);            /* Lista dei PCB liberi. */
HIDDEN pcb_t pcbFree_table[MAXPROC];    /* Tabella contenente tutti i PCB. */

void initPcbs() {
    for (int i = 0; i < MAXPROC; i++) list_add_tail(&(pcbFree_table[i].p_list), &pcbFree_h);    /* Scorre pcbFree_table e inserisce i p_list dei suoi elementi in pcbFree_h */
}

void freePcb(pcb_t* p) {
    if (p != NULL) list_add_tail(&(p->p_list), &pcbFree_h); /* Aggiunge p a pcbFree_h */
}

pcb_t* allocPcb() {
    if (list_empty(&pcbFree_h)) return NULL;                            /* Controllo lista vuota */
    else {
        pcb_PTR newPcb = container_of(pcbFree_h.next, pcb_t, p_list);   /* Viene trovato il primo PCB libero (successore dell'elemento sentinella) */
        list_del(pcbFree_h.next);                                       /* Rimozione del successore della sentinella */

        INIT_LIST_HEAD(&(newPcb->p_list));                              /* Inizializzazione delle liste */
        INIT_LIST_HEAD(&(newPcb->p_child));
        INIT_LIST_HEAD(&(newPcb->p_sib));

        newPcb->p_parent = NULL;                                        /* Inizializzazione del genitore e semaforo */
        newPcb->p_semAdd = NULL;

        (newPcb->p_s).entry_hi = 0;                                     /* Inizializzazione della struct p_s (viene utilizzato un ciclo per il campo gpr) */
        (newPcb->p_s).cause = 0;
        (newPcb->p_s).status = 0;
        (newPcb->p_s).pc_epc = 0;
        (newPcb->p_s).hi = 0;
        (newPcb->p_s).lo = 0;
        for (int i = 0; i < STATE_GPR_LEN; i++) (newPcb->p_s).gpr[i] = 0;

        newPcb->p_time = 0;

        return newPcb;
    }
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
