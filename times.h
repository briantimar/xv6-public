#include "types.h"

// holds process cycles elapsed, as well as total cycles elapsed, 
// since a process started exec()'ing
struct times {
    uint procticks;
    uint allticks;
};