#include "../h/asl.h"
#include "../h/pcb.h"

#define TRUE 1
#define FALSE 0

//array di semd di massima dimensione MAXPROC
HIDDEN semd_t semd_table[MAXPROC];
//Lista dei semafori liberi, inutilizzati
HIDDEN LIST_HEAD(semdFree_h); 
//Lista dei semafori attivi
HIDDEN LIST_HEAD(semd_h); 

int insertBlocked(int *semAdd, pcb_t *p) {
	semd_PTR res = getSemd(semAdd); 
	//Il semaforo si trova nella lista dei semafori attivi
	if (res != NULL){
		insertProcQ(&(res->s_procq),p); 
		p->p_semAdd = semAdd; 
		return FALSE; 
	}else{
		//Non ci sono semafori liberi
		if(list_empty(&semdFree_h)) return TRUE; 

		//prendo un semaforo dalla lista dei semafori liberi e lo rimuovo da tale lista
		res = container_of(list_prev(&semdFree_h),semd_t,s_link); 
		list_del(list_prev(&semdFree_h)); 

		//Inizializzazione dei campi del semaforo
		mkEmptyProcQ(&(res->s_procq)); 
		INIT_LIST_HEAD(&(res->s_link));
		//Inserisco p nella coda dei processi bloccati sul semaforo
		insertProcQ(&(res->s_procq),p); 
		p->p_semAdd = semAdd; 

		//Inserisco il semaforo nella ASL, lista ordinata in ordine crescente in base alla chiave semAdd
		semd_PTR iter; 
		list_for_each_entry(iter,&semd_h,s_link){
			if (res->s_key > iter->s_key){
				list_add(&res,&(iter->s_link)); 
				break; 
			}
		}
	}
	return FALSE; 
}

pcb_t *removeBlocked(int *semAdd) {
	semd_PTR res = getSemd(semAdd);
	struct list_head pcb;
	if (res != NULL){ //se il semd e' in ASL
		struct list_head e = res->s_procq;
		pcb = *list_next(&e); //copia il pcb per restituirlo dopo averlo cancellato
		list_del(list_next(&e));
		if(list_empty(&e)){ //se ha svuotato la coda rimuove anche il semd
			list_add_tail(&res, &semdFree_h);
			list_del(&res);
		}
	}else{
		return NULL;
	}
	return &pcb;
}

pcb_t *outBlocked(pcb_t *p) {
	/*
	semd_PTR res = getSemd(p->p_semAdd);
	if(is_proc_in_semd(res, p) == FALSE) //condizione di errore
		return NULL;
	
	pcb_t p2 = *p; //copia p per restituirlo dopo averlo cancellato
	list_del(p);

	if(list_empty(&p2)){ //se ha svuotato la lista rimuove anche il semd
		list_add_tail(&res, semdFree_h);
		list_del(&res);
	}
	return &p2;
	*/
	return NULL; 
}

pcb_t *headBlocked(int *semAdd) {
	semd_PTR res = getSemd(semAdd);
	struct list_head pcb;

	if (res != NULL){
		struct list_head e = res->s_procq;
		if(list_empty(&e)) //il semd e' in ASL ma la lista dei proc e' vuota
			return NULL;
		pcb = *list_next(&e);
	}else{ //il semd non e' in ASL
		return NULL;
	}
	return &pcb;
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
	struct list_head *iter;
	list_for_each(iter,&(s->s_procq)){
		if (iter == p)
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