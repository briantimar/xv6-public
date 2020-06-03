/// Process-status table
#ifndef _PSTAT_H_
#define _PSTAT_H_

#include "param.h"

struct pstat {
    int pid[NPROC]; // pid's
    int ticks[NPROC]; // number of clock ticks each process has accumulated.
    int state[NPROC]; // current state of each process
    char name[NPROC][16]; // process name
    int pages[NPROC]; // number of pages assigned to each process.
};

#endif // _PSTAT_H_