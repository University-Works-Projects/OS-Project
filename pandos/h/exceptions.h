#ifndef EXCEPTIONS
#define EXCEPTIONS

#include "types.h"
#include "pandos_const.h"
#include "pandos_types.h"
#include "pcb.h"
#include "initial.h"

#define INTERVAL_INDEX 0


/**
 * The exception handler is called when an exception occurs. It determines the cause of the exception
 * and calls the appropriate handler. 
 * 
 * The first thing the exception handler does is save the current time in the variable exception_time.
 * This is used to calculate the time spent in the exception handler. 
 * 
 * The exception handler then gets the current state of the processor from the BIOS data page. 
 * 
 * The cause of the exception is then determined by masking the cause register with the GETEXECCODE
 * macro. The cause is then shifted right by two bits to get the exception code. 
 * 
 * The exception code is then used to determine the cause of the exception. 
 */
void exception_handler(); 

/**
 * The function handles the system calls, updating the current_p and calling the scheduler if
 * necessary. 
 * 
 * The function extracts the system call code from the exception state and calls the appropriate
 * function. 
 * 
 * The function also handles the case in which the current_p is killed by the system call. In this
 * case, the function sets the curr_proc_killed flag to 1 and calls the scheduler. 
 * 
 * The function also handles the case in which the current_p is blocked by the system call. In this
 * case, the function sets the block_flag to 1 and calls the scheduler. 
 * 
 * The function also handles the case in which the current_p is not blocked or killed by the system
 * call. In this case, the function updates the current_p's state and calls the scheduler. 
 */
void syscall_handler(); 

// SYSCALLS
/**
 * NSYS1 - It creates a new process, initializes its fields and inserts it in the ready queue
 * 
 * @param a1_state the state of the process to be created
 * @param a2_p_prio the priority of the new process.
 * @param a3_p_support_struct a pointer to a support_t struct. This struct is used to store the
 * semaphore and the device semaphore that the process is waiting on.
 */
void create_process(state_t *a1_state, int a2_p_prio, support_t *a3_p_support_struct); 

/**
 * NSYS2 - It removes the process from the list of children of its parent, and then it terminates all its
 * descendants
 * 
 * @param a2_pid The process ID of the process to terminate.
 */
void terminate_process(int a1_pid);
/**
 * It removes all the children of the process passed as argument, and then it frees the process itself
 * 
 * @param old_proc the process to be terminated
 */
void terminate_all(pcb_PTR old_proc);

/**
 * NSYS3 & NSYS4 - This function perfoms either a P or a V (binary) operation based on p_flag value
 *  
 * @param a1_semaddr the address of the semaphore to be passed
 * @param block_flag a flag that indicates whether the current process has been blocked or not.
 * @param p_flag a flag that indicates whether the operation is a p or not.
 */
void sem_operation(int *a1_semaddr, int *block_flag, int p_flag);

/**
 * NSYS5 - It checks if the device is available, if so it performs the I/O operation, otherwise it blocks the
 * process
 * 
 * @param a1_cmdAddr the address of the device register to be accessed
 * @param a2_cmdValue the value to be written to the device register
 * @param block_flag if the process is blocked, it is set to 1, otherwise it is set to 0.
 */
void do_io(int *a1_cmdAddr, int a2_cmdValue, int *block_flag);

/**
 * NSYS6 - It returns the amount of time the process has been using the CPU
 */
void get_cpu_time();

/**
 * NSYS7 - It waits for the clock to tick
 * 
 * @param block_flag a pointer to a flag that indicates whether the process is blocked or not.
 */
void wait_for_clock(int *block_flag);

/**
 * NSYS8 - It returns the address of the support structure of the current process
 */
void get_support_data();

/**
 * NSYS9 - If the parent is 0, return the current process, otherwise return the parent process.
 * 
 * @param a1_parent 0 if you want the current process, 1 if you want the parent process
 */
void get_processor_id(int a1_parent);

/**
 * NSYS10 - The function `yield` is used to put the current process in the ready queue and to choose the next
 * process to be executed. 
 * 
 * @param block_flag if set to 1, the scheduler will be called to choose a new process to execute.
 * @param low_priority if set to 1, the scheduler will choose a low priority process to run.
 */
void yield(int *block_flag, int *low_priority);



/**
 * If the current process has a support structure, then we copy the exception state into the support
 * structure's exception state array, and then we load the context of the exception handler into the
 * CPU
 * 
 * @param index_value the index of the exception in the exception vector
 * @param exception_state the state of the process when the exception occurred
 */
void pass_up_or_die(int index_value, state_t *exception_state); 

/**
 * Insert to_insert pcb into the appropriate ready queue based on its priority
 * 
 * @param to_insert pcb to be inserted
 */
void ready_by_priority(pcb_PTR to_insert);

/**
 * @brief It takes the address of the page that caused the exception, it looks up the page table entry for
 * that page, and it writes the entry into the TLB.
 * One can place debug calls here, but not calls to print.
 */
void uTLB_RefillHandler(); 

#endif