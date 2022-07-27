#include "../h/sysSupport.h"

extern swap_t swap_pool[POOLSIZE];

void generalException_hanlder() {

    support_t* currProc_support = (support_t*) SYSCALL(GETSUPPORTPTR, 0, 0, 0);     /* SYS8 to get the supp struct's addrs of the curr proc */

    /* [Section 4.8] and [Section 3.7.2] (and [Section 3.4]) */
    // Why .[GENERALEXCEPT] ?
    int exeCode = currProc_support->sup_exceptState[GENERALEXCEPT].cause;       /*  */
    if (9 <= exeCode && exeCode >= 12)
        terminate(currProc_support);

    int syscode = currProc_support->sup_exceptState[GENERALEXCEPT].reg_a0;

    int status = currProc_support->sup_exceptState[GENERALEXCEPT].status;

    switch (syscode) {
        case GET_TOD: {
            get_tod(currProc_support);
            }
            break;
        case TERMINATE: {
            terminate(currProc_support);
            }
            break;
        case WRITEPRINTER: {
            write_to_printer();
            }
            break;
        case WRITETERMINAL: {
            write_to_terminal();
            }
            break;
        case READTERMINAL: {
            read_from_terminal();
            }
            break;
        default: {
            }
            break;
    }
    // Any return status is placed in v0
    currProc_support->sup_exceptState[GENERALEXCEPT].pc_epc += WORDLEN;
    LDST (&(currProc_support->sup_exceptState[GENERALEXCEPT]));
}

/* NSYS1 */
void get_tod (support_t* currProc_support) {
    unsigned int tmp;
    currProc_support->sup_exceptState[GENERALEXCEPT].reg_v0 = STCK(tmp);
}

/* NSYS2 */
void terminate (support_t* currProc_support) {
   /**
    * If the process to be terminated is currently holding mutual exclusion on
    * a Support Level semaphore (e.g. Swap Pool semaphore), mutual exclusion must
    * first be released (NSYS4) before invoking the Nucleus terminate command (NSYS2).
    */
}

/* NSYS3 */
void write_to_printer () {
    
}

/* NSYS4 */
void write_to_terminal () {
    
}

/* NSYS5 */
void read_from_terminal () {
    
}
