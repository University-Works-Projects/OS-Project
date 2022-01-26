#include "../h/asl.h"

#define TRUE 1
#define FALSE 0

//array di semd di massima dimensione MAXPROC
static semd_t semd_table[MAXPROC];
//Lista dei semafori liberi, inutilizzati
static struct list_head *semdFree_h;
//Lista dei semafori attivi
static struct list_head *semd_h; 

int insertBlocked(int *semAdd, pcb_t *p) {
	semd_PTR res = getSemd(semAdd);
	addokbuf("prova getSemd  \n");
	if(res != NULL){ //se il semd e' gia' attivo
		list_add_tail(p, &res->s_procq);
		p->p_semAdd = semAdd; 
		return FALSE;
	}else{
		if(!list_empty(&semdFree_h)){ //se in free c'e' almeno un semd
			res = container_of(list_prev(&semdFree_h),semd_t,s_link); 

			res->s_key = semAdd; //inizializza il semd
			INIT_LIST_HEAD(&(res->s_procq));
			list_add_tail(p, &res->s_procq);
			INIT_LIST_HEAD(&(res->s_link));

			list_add_tail(res, &semd_h); //aggiunge il semd in ASL
			list_del(res); //rimuove il semd dai free
			p->p_semAdd = semAdd; //aggiorno la chiave del semaforo su cui il processo è bloccato
			return FALSE;
		}
		else{ //i semd sono tutti occupati
			return TRUE;
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
			list_add_tail(&res, semdFree_h);
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
	INIT_LIST_HEAD(&semdFree_h); //inizializza ASL e free
	INIT_LIST_HEAD(&semd_h);
	for (int i=0; i<MAXPROC; i++){
		semd_t *e = &semd_table[i];
		INIT_LIST_HEAD(&e->s_link);
		INIT_LIST_HEAD(&e->s_procq);
		e->s_key = NULL; 
		list_add_tail(&e->s_link, &semdFree_h); //aggiunge i vari elementi di semb_table a free
	}
}

int is_proc_in_semd(semd_t *s, pcb_t *p){
	struct list_head *iter;
	list_for_each(iter,&s->s_procq){
		if (iter == p){
			return TRUE;
		}
	}
	return FALSE;
}

semd_PTR getSemd(int *key){ //ritorna il semd associato alla key
	if (list_empty(semd_h))
		return NULL; 
	struct list_head* iter;
	addokbuf("prova getSemd 2  \n");
	if (key != NULL)
		list_for_each(iter, semd_h){
			addokbuf("prova getSemd 3  \n");
			semd_PTR res = container_of(iter,semd_t,s_link);
			if (key == res->s_key)
				return res;
		}
	return NULL;
}
