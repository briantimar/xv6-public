#include "types.h"
#include "param.h"
#include "page.h"
#include "spinlock.h"
#include "defs.h"

// linked list of all frames
// currently there's no "backing". at some point might add swapping to disk for these.
// right now only used to keep track of page reference counts
struct {
    struct spinlock lock;
    struct pagebuf pages[NPAGE];
    // pointer to the mostly recently used page
    struct pagebuf* head;
    
} pagecache;


// setup code for the page cache. run this in main() 
void pageinit(void) {
    struct pagebuf *pg;
    initlock(&pagecache.lock, "pagecache");
    acquire(&pagecache.lock);
    pagecache.head = pagecache.pages;
    pagecache.head->prev = 0;

    for (pg=pagecache.pages; pg < pagecache.pages + (NPAGE-1); pg++) {
        (pg+1)->prev = pg;
        pg->next = pg+1;
        pg->used = 0;
    }
    pg->next = 0;
    pg->used = 0;
    release(&pagecache.lock);
}

// sets the given pagebuf to be the most recently used.
// call while holding pagelock.
void setMRU(struct pagebuf* pb) {
    // move to head of list
    if (pb->next != 0)
     {
        pb->next->prev = pb->prev;
        }
    if (pb->prev != 0)
     {
        pb->prev->next = pb->next;
        }
    pb->next = pagecache.head;
    pb->prev = 0;
    pagecache.head->prev = pb;
    pagecache.head = pb;
}

// returns pointer to a free page buffer
struct pagebuf* getfreebuf(void) {
    struct pagebuf* pb;
    acquire(&pagecache.lock);
    // walks the list of pages starting at most recently used.
    for(pb = pagecache.head; pb != 0; pb = pb->next) {
        if (~(pb->used)){
            pb->used = 1;
            setMRU(pb);
            release(&pagecache.lock);
            return pb;
        }
    }
    release(&pagecache.lock);
    panic("no free buffers");
}

// gets extant buffer with the given physical address, or zero if not in cache.
struct pagebuf* getbuf(uint physaddr){
    struct pagebuf* pb;
    acquire(&pagecache.lock);
    for (pb = pagecache.head; pb != 0; pb = pb->next)
    {
        if (pb->physaddr == physaddr && pb->used)
        {
            setMRU(pb);
            release(&pagecache.lock);
            return pb;
        }
    }
    release(&pagecache.lock);
    return 0;
}

// commits the buffer data
void writebuf(struct pagebuf* pb) {
    acquire(&pagecache.lock);
    setMRU(pb);
    release(&pagecache.lock);
}
