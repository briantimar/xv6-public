# Brian OS

Lots of inspiration drawn from the [https://github.com/remzi-arpacidusseau/ostep-projects](OSTEP xv6 projects).

## Things added

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