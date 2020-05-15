#include "types.h"
#include "user.h"

int main(int argc, char* argv []) {
    printf(1, "Running in process %d\n", getpid());
    
    // should be default ticket number
    printf(1, "TEST %d\n", gettickets());

    settickets(10);
    // better be what you just set
    printf(1, "TEST %d\n", gettickets());
    if (fork()==0) {
        // child should inherit the parent's ticket number
        printf(1, "TEST %d\n", gettickets());
    }
    wait();    
    exit();
}