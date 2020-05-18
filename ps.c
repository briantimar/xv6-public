#include "types.h"
#include "pstat.h"
#include "user.h"
#include "param.h"


void showps(struct pstat* ps) {
    int i;

    printf(1, "Process stats:\n-----------------\n");
    printf(1, "PID | name | state | ticket count | ticks\n---------------\n");
    
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
   
}

int main(int argc, char* argv[]) {

    struct pstat ps;
    
    showps(&ps);

    exit();
}