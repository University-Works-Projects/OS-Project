#include "../h/asl.h"
#include "../h/pcb.h"


HIDDEN semd_t semd_table[MAXPROC];					/* Array di semd di massima dimensione MAXPROC */
HIDDEN LIST_HEAD(semdFree_h); 						/* Lista dei semafori liberi, ma inutilizzati */
HIDDEN LIST_HEAD(semd_h); 							/* Lista dei semafori attivi */

int insertBlocked(int *semAdd, pcb_t *p) {
	semd_PTR res = getSemd(semAdd);
	if (res != NULL) {
		insertProcQ(&(res->s_procq), p); 
		p->p_semAdd = semAdd; 
		return FALSE; 
	} else {
		if (list_empty(&semdFree_h))										/* Se non ci sono semafori liberi */
			return TRUE; 
		else {
			res = container_of(list_prev(&semdFree_h), semd_t, s_link);		/* Si prende un elemento dalla lista dei semafori liberi */ 
			list_del(list_prev(&semdFree_h)); 								/* E lo si rimuove da tale lista */

			mkEmptyProcQ(&(res->s_procq)); 									/* Inizializzazione dei campi del semaforo */
			INIT_LIST_HEAD(&(res->s_link));
			res->s_key = semAdd;

			insertProcQ(&(res->s_procq), p);								/* Inserimento di p nella coda dei processi bloccati sul semaforo */
			p->p_semAdd = semAdd; 

			semd_PTR iter; 													/* Inserimento del semaforo nella ASL, lista ordinata in ordine crescente in base alla chiave semAdd */
			list_for_each_entry(iter, &semd_h, s_link){
				if (res->s_key > iter->s_key){
					list_add(&(res->s_link), &(iter->s_link)); 
					return FALSE; 
				}
			}
			list_add(&(res->s_link),&(semd_h)); 							/* Se ASL è vuota, si può inserire il descrittore in testa */
		}
	}
	return FALSE; 
}

pcb_t *removeBlocked(int *semAdd) {
	semd_PTR res = getSemd(semAdd); 
	//Il descrittore non è presente nella ASL
	if (res == NULL) return NULL; 
	
	//Prendo il primo PCB dalla coda dei processi bloccati e lo rimuovo
	pcb_PTR pcb = headProcQ(&(res->s_procq)); 
	if (pcb == NULL) return NULL; 
	pcb = removeProcQ(&(res->s_procq)); 

	//Verifica se bisogna rimuovere il descrittore del semaforo dalla ASL se è diventato libero
	if (emptyProcQ(&(res->s_procq))){
		list_del(&(res->s_link)); 
		list_add_tail(&(res->s_link),&semdFree_h); 
	}
	return pcb; 
}

pcb_t *outBlocked(pcb_t *p) {
	semd_PTR res = getSemd(p->p_semAdd); 
	//Condizione di errore, il PCB non si trova nella coda del semaforo
	if (is_proc_in_semd(res,p) == FALSE || res == NULL) return NULL; 
	
	//Rimuovo p dalla coda del semaforo su cui è bloccato
	p = outProcQ(&(res->s_procq),p); 

	//Verifica se bisogna rimuovere il descrittore del semaforo dalla ASL se è diventato libero
	if (emptyProcQ(&(res->s_procq))){
		list_del(&(res->s_link)); 
		list_add_tail(&(res->s_link),&semdFree_h); 
	}
	return p; 
}

pcb_t *headBlocked(int *semAdd) {
	semd_PTR res = getSemd(semAdd);
	//Il descrittore non è in ASL
	if (res == NULL || emptyProcQ(&(res->s_procq))) return NULL;

	return headProcQ(&res->s_procq); 
}

void initASL() {
	for (int i=0; i<MAXPROC; i++){
		semd_t *e = &semd_table[i];
		INIT_LIST_HEAD(&e->s_link);
		INIT_LIST_HEAD(&e->s_procq);
		e->s_key = NULL; 
		list_add_tail(&(e->s_link), &semdFree_h); //aggiunge i vari elementi di semb_table a free
	}
}

int is_proc_in_semd(semd_t *s, pcb_t *p){
	if (s == NULL) return FALSE; 
	struct list_head *iter;
	list_for_each(iter,&(s->s_procq)){
		if (container_of(iter,pcb_t,p_list) == p)
			return TRUE;
	}
	return FALSE;
}

semd_PTR getSemd(int *key){ //ritorna il semd associato alla key
	if (list_empty(&semd_h)) return NULL; 
	struct list_head* iter;
	if (key != NULL)
		list_for_each(iter, &semd_h){
			semd_PTR res = container_of(iter,semd_t,s_link);
			if (key == res->s_key)
				return res;
		}
	return NULL;
}