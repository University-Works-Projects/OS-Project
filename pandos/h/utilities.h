/**
 * Prende in input i puntatori a semd_t e pcb_t
 * Verifica che il processo sia nella coda del semaforo,
 * restituendo in TRUE(1) o FALSE(0) in caso contrario
 */
HIDDEN int is_proc_in_semd(semd_t *s, pcb_t *p);

/**
 * Data una key restituisce un puntatore al semd corrispondente
 * Se non esiste ritorna NULL
 */
HIDDEN semd_PTR getSemd(int *key);

/**
 * Dato un semaforo, se è libero lo rimuove da ASL e lo
 * inserisce in semdFree_h
 */
HIDDEN void checkEmpty (semd_PTR res);
