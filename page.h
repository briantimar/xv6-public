#include "types.h"

// a wrapper around a physical page frame
// does not hold data, just tracks references to extant pages.
struct pagebuf {
    // virtual address of the page = constant offset from physical address.
    char* vaddr;
    int used;
    int refcnt;
    struct pagebuf* next;
    struct pagebuf* prev;
};

int pagecount(void);

// API for copy-on-write allocation
// returns a new page with reference count 1
char* getpage(void);
// lazily frees a new page; this may or may not free the underlying physical memory
int freepage(char*);
// marks existing page as copied, returns new ref count
int lazycopy(char*);
// copy a physical page if it has multiple refs
char* forkpage(char*);
// returns number of references to a given page
int refcount(char*);