#include "asl.h"
#include "./phase1_prof/pandos_const.h"
#include "./phase1_prof/listx.h"
#include "./phase1_prof/pandos_types.h"

#define TRUE 1
#define FALSE 0

static semd_t semd_table[MAXPROC];
static struct list_head *semdFree_h;
static struct list_head *semd_h; 



semd_PTR getSemd(int *key){
	struct list_head* iter;
	list_for_each(iter, semd_h){
		semd_PTR temp = container_of(iter,semd_t,s_link);
		if (key == temp->s_key)
			return temp;
	}
	return NULL;
}

int insertBlocked(int *semAdd, pcb_t *p) {
	//if semd in asl
		//inserisce
		//return false
	//else
		//if puo' allocare
			//allocare
			//return FALSE
		//else
			//return true
	semd_PTR res = getSemd(semAdd);
	if(res != NULL){
		list_add_tail(p, &res->s_procq);
		return FALSE;
	}
	else{
		if(!list_empty(&semdFree_h)){
			semd_t *e = list_prev(&semdFree_h);
			e->s_key = semAdd;
			INIT_LIST_HEAD(&(e->s_procq));
			list_add_tail(p, &e->s_procq);
			INIT_LIST_HEAD(&(e->s_link));
			list_add_tail(e, &semd_h);
			list_del(e);
			return FALSE;
		}
		else{
			return TRUE;
		}
	}
	return FALSE;
}

pcb_t *removeBlocked(int *semAdd) {
	semd_PTR res = getSemd(semAdd);
	struct list_head pcb;
	if (res != NULL){
		struct list_head e = res->s_procq;
		pcb = *list_next(&e);
		list_del(list_next(&e));
		if(list_empty(&e)){
			list_add_tail(&res, semdFree_h);
			list_del(&res);
		}
	}else{
		return NULL;
	}
	return &pcb;
}

int foo(semd_t *s, pcb_t *p){
	struct list_head *iter;
	list_for_each(iter,&s->s_procq){
		if (iter == p){
			return TRUE;
		}
	}
	return FALSE;
}

pcb_t *outBlocked(pcb_t *p) {
	semd_PTR res = getSemd(p->p_semAdd);
	if(foo(res, p) == FALSE)
		return NULL;
	
	pcb_t p2 = *p;
	list_del(p);
	if(list_empty(&p2)){
		list_add_tail(&res, semdFree_h);
		list_del(&res);
	}
	return &p2;
}

pcb_t *headBlocked(int *semAdd) {
	semd_PTR res = getSemd(semAdd);
	struct list_head pcb;
	if (res != NULL){
		struct list_head e = res->s_procq;
		if(list_empty(&e))
			return NULL;
		pcb = *list_next(&e);
	}else{
		return NULL;
	}
	return &pcb;
}

/*
Inizializza la lista dei semdFree in
modo da contenere tutti gli elementi
della semdTable. Questo metodo
viene invocato una volta sola durante
l’inizializzazione della struttura dati.
*/
void initASL() {
	//if(&semdFree_h == NULL){//da testare
	INIT_LIST_HEAD(&semdFree_h);
	INIT_LIST_HEAD(&semd_h);
	for (int i=0; i<MAXPROC; i++){
		semd_t e = semd_table[i];
		list_add_tail(&e, &semdFree_h);
	}
	//}
}
