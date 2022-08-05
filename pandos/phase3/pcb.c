#include "../h/pcb.h"

// Lista dei PCB liberi
HIDDEN LIST_HEAD(pcbFree_h);            
// Tabella contenente tutti i PCB
HIDDEN pcb_t pcbFree_table[MAXPROC];    

void initPcbs() {
    for (int i = 0; i < MAXPROC; i++)                                       
        // Inserisce i p_list in pcbFree_h
        list_add_tail(&(pcbFree_table[i].p_list), &pcbFree_h);              
}

void freePcb(pcb_t* p) {
    if (p != NULL)
        list_add_tail(&(p->p_list), &pcbFree_h);
}

pcb_t* allocPcb() {
    if (list_empty(&pcbFree_h))
        return NULL;
    else {
        // Si prende il primo PCB libero (ovvero il successore della sentinella)
        pcb_PTR newPcb = container_of(pcbFree_h.next, pcb_t, p_list);       
        list_del(pcbFree_h.next);

        // Inizializzazione delle liste e dei campi
        INIT_LIST_HEAD(&(newPcb->p_list));                                  
        INIT_LIST_HEAD(&(newPcb->p_child));
        INIT_LIST_HEAD(&(newPcb->p_sib));

        newPcb->p_parent = NULL;
        newPcb->p_semAdd = NULL;

        // Inizializzazione della struct p_s e p_time
        (newPcb->p_s).entry_hi = 0;                                         
        (newPcb->p_s).cause = 0;
        (newPcb->p_s).status = 0;
        (newPcb->p_s).lo = 0;
        for (int i = 0; i < STATE_GPR_LEN; i++)
            (newPcb->p_s).gpr[i] = 0;
        (newPcb->p_s).pc_epc = 0;
        (newPcb->p_s).hi = 0;
        newPcb->p_time = 0;
        
        // Inizializzazione dei campi rimantenti
        newPcb->p_prio = 0;                                                 
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
        // Ritorna la coda, ovvero l'elemento successivo (per via dell'inserimento in coda) alla sentinella
        return container_of(head->next, pcb_t, p_list);                 
}

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
        // Per ogni PCB della coda dei processi head
        list_for_each_entry(iter, head, p_list) {                       
            if (iter == p) {
                // Rimozione di p da head
                list_del(&(p->p_list));                                 
                return p;
            }
        }
        // Caso in cui non venga trovato p in head
        return NULL;                                                    
    }
}

int emptyChild(pcb_t* p) {
    return list_empty(&(p->p_child));
}

void insertChild(pcb_t* prnt, pcb_t* p) {
    if (p != NULL && prnt != NULL) {
        // Per aggiungere p tra i figli di prnt si passa: p->p_sib (ovvero i fratelli di p) e prnt->p_child (ovvero i figli di prnt)
        list_add_tail(&(p->p_sib), &(prnt->p_child));                       
        p->p_parent = prnt;
    }
}

pcb_t* removeChild(pcb_t* p) {
    // Controllo list_empty su p_child, ovvero: se p ha dei figli...
    if (list_empty(&(p->p_child)))                                      
        return NULL;
    else {
        // Si prende il primo figlio tramite container_of
        pcb_t* tmp = container_of((p->p_child).next, pcb_t, p_sib);     
        // E lo si rimuove dalla lista dei fratelli
        list_del(&(tmp->p_sib));                                        
        tmp->p_parent = NULL;
        return tmp;
    }
}

pcb_t* outChild(pcb_t* p) {
    if (p->p_parent == NULL)                                            
        // Se p non ha un padre
        return NULL;
    else {
        // Rimozione dalla lista dei fratelli
        list_del(&(p->p_sib));                                          
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