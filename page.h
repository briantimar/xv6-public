#include "types.h"

// a wrapper around a physical page frame
// does not hold data, just tracks references to extant pages.
struct pagebuf {
    uint physaddr;
    int used;
    int refcnt;
    struct pagebuf* next;
    struct pagebuf* prev;
};

struct pagebuf* getbuf(uint);
void writebuf(struct pagebuf*);
struct pagebuf* getfreebuf(void);
void releasebuf(struct pagebuf*);
void pbfree( uint);
int pagecount(void);

