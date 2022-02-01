#include "../h/asl.h"
#include "../h/pcb.h"

HIDDEN semd_t semd_table[MAXPROC];					/* Array di semd di massima dimensione MAXPROC */
HIDDEN LIST_HEAD(semdFree_h); 						/* Lista dei semafori liberi, ma inutilizzati */
HIDDEN LIST_HEAD(semd_h); 							/* Lista dei semafori attivi */


int insertBlocked(int *semAdd, pcb_t *p) {
	semd_PTR res = getSemd(semAdd);
	if (res != NULL) {														/*Il semaforo si trova nella ASL*/
		insertProcQ(&(res->s_procq), p); 
		p->p_semAdd = semAdd; 
		return FALSE; 
	} else {
		if (list_empty(&semdFree_h))										/* Non vi sono semafori liberi */
			return TRUE; 
		else {
			res = container_of(list_prev(&semdFree_h), semd_t, s_link);		/* Si prende un elemento dalla lista dei semafori liberi, rimuovendolo*/ 
			list_del(list_prev(&semdFree_h)); 								

			res->s_key = semAdd;											/* Si assegna la nuova chiave del semaforo */

			insertProcQ(&(res->s_procq), p);								/* Si inserisce p nella coda dei processi bloccati sul semaforo */
			p->p_semAdd = semAdd; 

			semd_PTR iter; 													/* Si inserisce il semaforo nella ASL, lista ordinata in ordine crescente in base alla chiave semAdd */
			list_for_each_entry(iter, &semd_h, s_link){
				if (res->s_key > iter->s_key){
					list_add(&(res->s_link), &(iter->s_link)); 
					return FALSE; 
				}
			}
			list_add(&(res->s_link),&(semd_h)); 							/* Se ASL è vuota, si può inserire il semaforo in testa a ASL*/
		}
	}
	return FALSE; 
}

pcb_t *removeBlocked(int *semAdd) {
	semd_PTR res = getSemd(semAdd); 
	
	if (res == NULL)  														/* Il semaforo non è presente nella ASL, quindi non ha PCB blocati su esso*/
		return NULL;	
	pcb_PTR pcb = headProcQ(&(res->s_procq));								/* Si prende il primo PCB dalla coda dei processi bloccati del semaforo lo si rimuove */ 
	if (pcb == NULL) 
		return NULL; 
	pcb = removeProcQ(&(res->s_procq)); 

	if (emptyProcQ(&(res->s_procq))){										/* Se il semaforo è diventato libero */
		list_del(&(res->s_link)); 
		list_add_tail(&(res->s_link),&semdFree_h); 
	}
	return pcb; 
}

pcb_t *outBlocked(pcb_t *p) {
	semd_PTR res = getSemd(p->p_semAdd); 
	if (is_proc_in_semd(res,p) == FALSE || res == NULL) 					/* Il PCB non si trova nella coda del suo semaforo o il PCB non ha un semaforo valido associato */ 
		return NULL; 

	p = outProcQ(&(res->s_procq),p); 										/* Si rimuove il PCB dalla coda del semaforo su cui è bloccato */

	if (emptyProcQ(&(res->s_procq))){										/* Se il semaforo è diventato libero */
		list_del(&(res->s_link)); 
		list_add_tail(&(res->s_link),&semdFree_h); 
	}
	return p; 
}

pcb_t *headBlocked(int *semAdd) {
	semd_PTR res = getSemd(semAdd);
	//Il descrittore non è in ASL
	if (res == NULL || emptyProcQ(&(res->s_procq))) 						/* Il semaforo non è presente nella ASL */
		return NULL; 
	return headProcQ(&res->s_procq); 
}

void initASL() {
	for (int i=0; i<MAXPROC; i++){
		semd_t *e = &semd_table[i];
		
		INIT_LIST_HEAD(&e->s_link);											/* Si inizializzano i campi del descrittore del semaforo */
		mkEmptyProcQ(&e->s_procq);
		e->s_key = NULL;

		list_add_tail(&(e->s_link), &semdFree_h);							/* Si aggiunge il semaforo alla lista dei semafori liberi */ 
	}
}

int is_proc_in_semd(semd_t *s, pcb_t *p){
	if (s == NULL) 
		return FALSE; 														/* Caso base */
	struct list_head *iter;
	list_for_each(iter,&(s->s_procq)){
		if (container_of(iter,pcb_t,p_list) == p)							/* Se il PCB si trova nella lista dei processi bloccati sul semaforo */
			return TRUE;
	}
	return FALSE;
}

semd_PTR getSemd(int *key){ 
	if (list_empty(&semd_h) || key == NULL) 								/* Caso base */
		return NULL; 
	struct list_head* iter;
	list_for_each(iter, &semd_h){
		semd_PTR res = container_of(iter,semd_t,s_link);
		if (key == res->s_key)												/* Se il semaforo associato alla key si trova nella ASL */
			return res;
	}
	return NULL;
}