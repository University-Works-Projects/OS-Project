#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/exceptions.h"
#include "../h/interrupts.h"
#include "../h/scheduler.h"

#define N_SUBDEVICE 1

int semaphores_subDevice[N_SUBDEVICE];
int liveProcesses = 0, blockedProcesses = 0;
pcb_t *readyProcesses = NULL, *currentProcess = NULL;

int main () {

    intPcbs();
    intSemd();

    return 0;
}