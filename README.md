# Brian OS

Lots of inspiration drawn from the [OSTEP xv6 projects](https://github.com/remzi-arpacidusseau/ostep-projects).

## Things added

* Threads, accessible to users via `thread_create()` and `join()` library functions.
* Lottery scheduling for processes, as per OSTEP. Users can set process priorities by assigning tickets via system call.
* `getpstat` system call and corresponding `ps` user program to get summary of current process statuses.
* Unmapped first page, to produce pagefaults when null pointers are dereferenced
* `mprotect()` and `munprotect()` system calls to toggle page write permissions
* Automatic read-only permission for any page loaded into process memory that contains only program text.


## Notes

### Summary of the address space

It's essentially the same as the standard xv6 address space, but shifted up by one page. RO protections are added to all pages which contain only program text. If there are NTEXT of these, and NELF pages total in the executable, the layout looks like this:

0x0000 - 0x1000: unmapped

0x1000 - NTEXT * 0x1000: RO text

NTEXT * 0x1000 - NELF * 0x1000: RW program data (some .text, as well as the data sections)

NELF * 0x1000 - (NELF+1) * 0x1000: guard page, user-inaccessible

(NELF+1) * 0x1000 - (NELF+2) * 0x1000: stack

### How threading is done.

The API is mostly borrowed from the OSTEP project description. Under the head, each thread is implemented as an xv6 process and lives in the process table - from the point of view of the scheduler, it's indistinguishable from a process. The stack thread lives in user memory, obtained via `malloc()`. The thread is initialized in the `clone()` system call, which allocates a kernel stack to the new thread and copies the page directory of the parent process. The stack is set up with a single `void *` argument, and the instruction pointer is set to the function pointer passed in by the user before the thread is handed off to the scheduler.

Threads, like xv6 processes, must terminate via `exit()`. Zombie threads are collected by `join()`, which provides the stack pointer of each dead thread to the user for cleanup.

Spin locks are added to the `umalloc` library, to prevent race conditions when multiple threads in the same address space need to allocated memory.

## TODO

* `clear`
* better `ps` (address space summary?)
* `mmap`
* demand zeroing, copy-on-write for pages allocated to processes
* a `time` utility
* swap space and a page cache

## known issues

* the random number generator used for the lottery-ticket scheduling can overflow
* interrupts are disabled by default when acquiring spinlocks, which means clock-tick values based on timer interrupt counts are only approximate.