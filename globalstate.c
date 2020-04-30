#include "spinlock.h"
#include "defs.h"
struct counter
{
    struct spinlock lock;
    int count;
};

// initialize a counter with a lock
void init_counter(struct counter* ct, char* name) {
    initlock(&(ct->lock), name);
    ct->count =0;
}

void increment_counter(struct counter* ct) {
    acquire(&(ct->lock));
    ct->count += 1;
    release(&(ct->lock));
}

int read_counter(struct counter* ct) {
    int _ct;
    acquire(&(ct->lock));
    _ct = ct->count;
    release(&(ct->lock));
    return _ct;
}

// number of calls made to read() - from any process.
struct counter numread;

// For now, only one counter, so just leave these here.

void init_numread(void) {
    init_counter(&numread, "getreadlock");
}

void increment_numread(void) {
    increment_counter(&numread);
}

int get_numread(void) {
    return read_counter(&numread);
}

