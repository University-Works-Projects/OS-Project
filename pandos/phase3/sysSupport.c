#include "../h/sysSupport.h"

void generalException_hanlder() {

    support_t* currProc_support = (support_t*) SYSCALL(GETSUPPORTPTR, 0, 0, 0);     /* SYS8 to get the supp struct's addrs of the curr proc */

    /* [Section 4.8] and [Section 3.7.2] (and [Section 3.4]) */
    // Why .[GENERALEXCEPT] ?
    int exeCode = currProc_support->sup_exceptState[GENERALEXCEPT].cause;       /*  */
    if (9 <= exeCode && exeCode >= 12)
        terminate();

    int syscode = currProc_support->sup_exceptState[GENERALEXCEPT].reg_a0;

    switch (syscode) {
        case GET_TOD: {
            get_tod();
        }
        case TERMINATE: {
            terminate();
        }
        case WRITEPRINTER: {
            write_to_printer();
        }
        case WRITETERMINAL: {
            write_to_terminal();
        }
        case READTERMINAL: {
            read_from_terminal();
        }
        default: {

        }
    }

}

void get_tod () {
    
}

void terminate () {
   /**
    * If the process to be terminated is currently holding mutual exclusion on
    * a Support Level semaphore (e.g. Swap Pool semaphore), mutual exclusion must
    * first be released (NSYS4) before invoking the Nucleus terminate command (NSYS2).
    */
}

void write_to_printer () {
    
}

void write_to_terminal () {
    
}

void read_from_terminal () {
    
}
