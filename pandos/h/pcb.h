#ifndef PCB_H
#define PCB_H

#include "pandos_types.h"

/**
 *  Inizializza la pcbFree in modo da contenere tutti gli elementi della
 *  pcbFree_table. Questo metodo deve essere chiamato una volta sola in fase
 *  di inizializzazione della struttura dati.
 */
void initPcbs();

/**
 * Inserisce il PCB puntato da p nella lista dei PCB liberi (pcbFree_h)
 */
void freePcb(pcb_t* p);

/**
 * Restituisce NULL se la pcbFree_h è vuota. Altrimenti rimuove un elemento
 * dalla pcbFree, inizializza tutti i campi (NULL/0) e restituisce
 * l’elemento rimosso.
 */
pcb_t* allocPcb();

/**
 * Crea una lista di PCB, inizializzandola
 * come lista vuota (i.e. restituisce NULL).
 */
void mkEmptyProcQ(struct list_head* head);

/**
 * Restituisce TRUE se la lista puntata da head è vuota, FALSE altrimenti.
 */
int emptyProcQ(struct list_head* head);

/**
 * Inserisce l’elemento puntato da p nella coda dei processi tp. La doppia
 * indirezione su tp serve per poter inserire p come ultimo elemento della coda.
 */
void insertProcQ(struct list_head* head, pcb_t* p);

/**
 * Restituisce l’elemento in fondo alla coda dei processi tp, SENZA RIMUOVERLO.
 * Ritorna NULL se la coda non ha elementi.
 */
pcb_t* headProcQ(struct list_head* head);

/**
 * Rimuove l’elemento piu’ vecchio dalla coda tp.
 * Ritorna NULL se la coda è vuota, altrimenti ritorna il puntatore
 * all’elemento rimosso dalla lista.
 */
pcb_t* removeProcQ(struct list_head* head);

/**
 * Rimuove il PCB puntato da p dalla coda dei processi puntata da tp.
 * Se p non è presente nella coda, restituisce NULL (p può trovarsi in una
 * posizione arbitraria della coda).
 */
pcb_t* outProcQ(struct list_head* head, pcb_t* p);

/**
 * Restituisce TRUE se il PCB puntato da p non ha figli, FALSE altrimenti.
 */
int emptyChild(pcb_t* p);

/**
 * Inserisce il PCB puntato da p come figlio del PCB puntato da prnt.
 */
void insertChild(pcb_t* prnt, pcb_t* p);

/**
 * Rimuove il primo figlio del PCB puntato da p.
 * Se p non ha figli, restituisce NULL.
 */
pcb_t* removeChild(pcb_t* p);

/**
 * Rimuove il PCB puntato da p dalla lista dei figli del padre.
 * Se il PCB puntato da p non ha un padre, restituisce NULL, altrimenti
 * restituisce l’elemento rimosso (cioè p).
 * A differenza della removeChild, p può trovarsi in una posizione arbitraria
 * (ossia non è necessariamente il primo figlio del padre).
 */
pcb_t* outChild(pcb_t* p);

// Questa funzione si occupa di copiare tutti i campi dello stato b nello stato a
void copy_state(state_t *a, state_t *b); 


#endif
