#include "types.h"
#include "pstat.h"
#include "user.h"
#include "param.h"

// char* safestrcpy(char*, char*, int);

void showps(struct pstat* ps, int delay) {
    int i;

    printf(1, "Process stats:\n-----------------\n");
    printf(1, "PID | name | state | ticket count | ticks\n---------------\n");
    
    while (1) {
        if ((getpstat(ps)) < 0)
         {
            printf(2, "Unable to load ptstat\n");
            exit();
         }
        for (i=0; i<NPROC; i++) {
            if (ps->state[i] > 0) {
                printf(1, "%d | %s | %d | %d | %d \n", ps->pid[i],
                                                    ps->name[i], 
                                                    ps->state[i],
                                                        ps->tickets[i], 
                                                        ps->ticks[i]);
            }
        }
     sleep(delay);
    }
   
}

int main(int argc, char* argv[]) {

    int delay = 10;
    struct pstat ps;
    
    showps(&ps, delay);

    exit();
}