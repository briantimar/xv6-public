#include "types.h"
#include "param.h"
#include "mmu.h"
#include "page.h"
#include "spinlock.h"
#include "defs.h"

// implements the Copy-on-write page mechanism
// users can:
// get a fresh physical page with getpage()
// 'free' a page with freepage()
// lazily copy a page with lazycopy()

// _locked prefix means the function should only be called while holding the pagecache lock.


// linked list of all frames
// currently there's no "backing". at some point might add swapping to disk for these.
// right now only used to keep track of page reference counts
struct {
    struct spinlock lock;
    int uselock;
    struct pagebuf pages[NPAGE];
    // pointer to the mostly recently used page
    struct pagebuf* head;
    // number of pages currently allocated
    int nalloc;
    
} pagecache;

// acquire or release the lock, only if necessary

static inline void acq(void)
{
    if (pagecache.uselock)
        acquire(&pagecache.lock);
}
static inline void rel(void)
{
    if (pagecache.uselock)
        release(&pagecache.lock);
}



// setup code for the page cache. run this in main(), before first kernel pgdir setup on boot processor
void pageinit1(void) {
    struct pagebuf *pg;
    initlock(&pagecache.lock, "pagecache");
    pagecache.head = pagecache.pages;
    pagecache.head->prev = 0;
    pagecache.nalloc = 0;
    // for the kernel setup, don't want to lock initially when cpu's haven't been configured
    pagecache.uselock = 0;

    for (pg=pagecache.pages; pg < pagecache.pages + (NPAGE-1); pg++) {
        (pg+1)->prev = pg;
        pg->next = pg+1;
        pg->used = 0;
    }
    pg->next = 0;
    pg->used = 0;    
}

// then run this after the other cpus have been started
void pageinit2(void) {
    pagecache.uselock = 1;
}


// sets the given pagebuf to be the most recently used.
// call while holding pagelock.
static void setMRU_locked(struct pagebuf* pb) {
           
    if (pb == pagecache.head){
        return;
    }
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

// returns pointer to a free page buffer, with ref count set to 1.
// marks it as used with ref ct 1
// call while holding pagecache lock
// returns 0 if no buffers available.
static struct pagebuf* findfreebuf_locked(void) {
    struct pagebuf* pb;
    // walks the list of pages starting at most recently used.
    for(pb = pagecache.head; pb != 0; pb = pb->next) {
        if ((pb->used)==0){
            return pb;
        }
    }
    return 0;
}

// finds extant buffer with the given physical address, or zero if not in cache.
// only returns a pointer - does not modify buffer.
static struct pagebuf* findbuf_locked(char* vaddr){
    struct pagebuf* pb;
    for (pb = pagecache.head; pb != 0; pb = pb->next)
    {
        if (pb->vaddr == vaddr && (pb->used ==1))
        {
            return pb;
        }
    }
    return 0;
}

// mark a buffer as unused and update the global allocation count accordingly
// call while holding pagecache lock
static void clearbuf_locked(struct pagebuf* pb) {
    pb->used = 0;
    pb->refcnt = 0;
    pb->vaddr = 0;
    pagecache.nalloc--;
}

// call while holding pagecache lock
// kallocs a new page, places it into a buffer
static struct pagebuf* getnewpage_locked(void)
{
    char* va;
    struct pagebuf *pb;
    if ((va = kalloc()) == 0)
        return 0;
    memset(va, 0, PGSIZE);
    if ((pb = findfreebuf_locked()) == 0)
    {
        kfree(va);
        return 0;
    }
    // write into new page buffer
    pb->used = 1;
    pb->refcnt = 1;
    pb->vaddr = va;
    pagecache.nalloc++;
    return pb;
}

// returns pointer to new page
// under the hood, this allocates a new physical page and adds it to the page buffer.
// returns 0 if allocation fails
char*  getpage(void) {
    char* va;
    struct pagebuf* pb;

    acq();
    if ((pb = getnewpage_locked())==0) {
        rel();
        return 0;
    }
    setMRU_locked(pb);
    va = pb->vaddr;
    rel();
    return va;
}

/// lazily free the page with the given address.
/// ref count is decreased and page is returned to the page buf
/// returns the remaining reference count.
int freepage(char* va) {
    struct pagebuf* pb;
    int refct = 0;
    acq();
    if ((pb = findbuf_locked(va)) != 0) {
        if ((pb->refcnt) == 0) {
            panic("freepage: ref count zero.");
        }
        if ((refct = --(pb->refcnt)) == 0) {
            // free the physical memory
            kfree(pb->vaddr);
            // clear from the page cache
            clearbuf_locked(pb);
        }
        setMRU_locked(pb);
    }
    rel();
    return refct;
}

// lazily copy the given page by incrementing its ref count.
// returns the new ref count.
int lazycopy(char* va) {
    struct pagebuf* pb;
    int refct;
    acq();
    if ((pb = findbuf_locked(va)) == 0) {
        rel();
        return 0;
    }
    refct = ++pb->refcnt;
    rel();
    return refct;
}

// copies a new page when necessary:
// if the page has a single reference, nothing is done; its address is returned.
// if the page has multiple references, copies its data into a new physical page and returns its address.
// returns 0 upon failure.
char* forkpage(char* va) {
    struct pagebuf *pb, *pbnew;
    acq();
    if ((pb = findbuf_locked(va))==0) {
        rel();
        return 0;
    }
    setMRU_locked(pb);
    if (pb->refcnt == 0) {
        rel();
        panic("forkpage: zero ref ct");
    }

    if (pb->refcnt == 1) {
        // for single ref, nothing to do.
        rel();
        return va;
    }
    else {
        // remove ref to the old page and get a new one.
        if ((pbnew = getnewpage_locked()) == 0) {
            rel();
            return 0;
        }
        pb->refcnt--;
        // copy the old page over.
        memmove(pbnew->vaddr, pb->vaddr, PGSIZE);
        setMRU_locked(pbnew);
        // user gets the new address
        rel();
        return pbnew->vaddr;
    }
}

// returns number of extant references to the given page
int refcount(char* va) {
    struct pagebuf* pb;
    int ct;
    acq();
    if ((pb = findbuf_locked(va)) ==0){
        rel();
        return 0;
    }
    ct = pb->refcnt;
    rel();
    return ct;
}

// number of pages allocated
int pagecount(void)
{
    int ct;
    acq();
    ct = pagecache.nalloc;
    rel();
    return ct;
}