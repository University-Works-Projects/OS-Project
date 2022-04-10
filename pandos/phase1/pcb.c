#include "../h/pcb.h"

HIDDEN LIST_HEAD(pcbFree_h);            /* Lista dei PCB liberi */
HIDDEN pcb_t pcbFree_table[MAXPROC];    /* Tabella contenente tutti i PCB */

void initPcbs() {
    for (int i = 0; i < MAXPROC; i++)                                       /* Scorrendo la pcbFre_table */
        list_add_tail(&(pcbFree_table[i].p_list), &pcbFree_h);              /* Inserisce i p_list in pcbFree_h */
}

void freePcb(pcb_t* p) {
    if (p != NULL)
        list_add_tail(&(p->p_list), &pcbFree_h);
}

pcb_t* allocPcb() {
    if (list_empty(&pcbFree_h))
        return NULL;
    else {
        pcb_PTR newPcb = container_of(pcbFree_h.next, pcb_t, p_list);       /* Si prende il primo PCB libero (ovvero il successore della sentinella) */
        list_del(pcbFree_h.next);

        INIT_LIST_HEAD(&(newPcb->p_list));                                  /* Inizializzazione delle liste e dei campi*/
        INIT_LIST_HEAD(&(newPcb->p_child));
        INIT_LIST_HEAD(&(newPcb->p_sib));

        newPcb->p_parent = NULL;
        newPcb->p_semAdd = NULL;

        (newPcb->p_s).entry_hi = 0;                                         /* Inizializzazione della struct p_s e p_time */
        (newPcb->p_s).cause = 0;
        (newPcb->p_s).status = 0;
        (newPcb->p_s).lo = 0;
        for (int i = 0; i < STATE_GPR_LEN; i++)
            (newPcb->p_s).gpr[i] = 0;
        (newPcb->p_s).pc_epc = 0;
        (newPcb->p_s).hi = 0;
        newPcb->p_time = 0;
        
        /* Per default, la priorità del processo è bassa (= 0) */
        newPcb->p_prio = 0;                                                 /* Inizializzazione dei campi rimantenti */
        newPcb->p_pid = 0;
        newPcb->p_supportStruct = NULL; 

        return newPcb;
    }
}

void mkEmptyProcQ(struct list_head* head) {
    if (head != NULL) INIT_LIST_HEAD(head);
}

int emptyProcQ(struct list_head* head) {
    return list_empty(head);
}

void insertProcQ(struct list_head* head, pcb_t* p) {
    list_add_tail(&(p->p_list), head);
}

pcb_t* headProcQ(struct list_head* head) {
    if (list_empty(head))
        return NULL;
    else
        return container_of(head->next, pcb_t, p_list);                 /* Ritorna la coda, ovvero l'elemento successivo (per via dell'inserimento in coda) alla sentinella */
}

/** WARNING:
 * Credo sia da capire se cambiare la semantica di queste funzioni (per semplificarci la vita
 * nella fase 2) non comporti a problemi causati dal fatto che umps3 si aspetti funzionino come 
 * descritte nella fase 1. Male che vada siamo costretti a crearne delle nuove.
 */
pcb_t* removeProcQ(struct list_head* head) {
    if (list_empty(head))
        return NULL;
    else {
        pcb_t* oldestPcb = container_of(head->next, pcb_t, p_list);
        list_del(head->next);
        return oldestPcb;
    }
}

pcb_t* outProcQ(struct list_head* head, pcb_t* p) {
    if (list_empty(head))
        return NULL;
    else {
        pcb_t* iter;
        list_for_each_entry(iter, head, p_list) {                       /* Per ogni PCB della coda dei processi head */
            if (iter == p) {
                list_del(&(p->p_list));                                 /* Rimozione di p da head */
                return p;
            }
        }
        return NULL;                                                    /* Caso in cui non venga trovato p in head */
    }
}

int emptyChild(pcb_t* p) {
    return list_empty(&(p->p_child));
}

void insertChild(pcb_t* prnt, pcb_t* p) {
    if (p != NULL && prnt != NULL) {
        list_add_tail(&(p->p_sib), &(prnt->p_child));                       /* Per aggiungere p tra i figli di prnt si passa: p->p_sib (ovvero i fratelli di p) e prnt->p_child (ovvero i figli di prnt) */
        p->p_parent = prnt;
    }
}

pcb_t* removeChild(pcb_t* p) {
    if (list_empty(&(p->p_child)))                                      /* Controllo list_empty su p_child, ovvero: se p ha dei figli... */
        return NULL;
    else {
        pcb_t* tmp = container_of((p->p_child).next, pcb_t, p_sib);     /* Si prende il primo figlio tramite container_of */
        list_del(&(tmp->p_sib));                                        /* E lo si rimuove dalla lista dei fratelli */
        tmp->p_parent = NULL;
        return tmp;
    }
}

pcb_t* outChild(pcb_t* p) {
    if (p->p_parent == NULL)                                            /* Se p non ha un padre */
        return NULL;
    else {
        list_del(&(p->p_sib));                                          /* Rimozione dalla lista dei fratelli */
        p->p_parent = NULL;
        return p;
    }
}

void copy_state(state_t *a, state_t *b) {
    a->entry_hi = b->entry_hi;
    a->cause = b->cause;
    a->status = b->status;
    a->pc_epc = b->pc_epc;
    for (int i = 0; i < STATE_GPR_LEN; i++)
        a->gpr[i] = b->gpr[i];
    a->hi = b->hi;
    a->lo = b->lo;
}