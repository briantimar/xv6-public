/// Process-status table
#ifndef _PSTAT_H_
#define _PSTAT_H_

#include "param.h"

struct pstat {
    int inuse[NPROC]; // whether the corresponding proctable slot is in use (0 or 1)
    int tickets[NPROC]; // number of tickets assigned to a given process
    int pid[NPROC]; // pid's
    int ticks[NPROC]; // number of clock ticks each process has accumulated.
}

#endif // _PSTAT_H_