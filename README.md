### Brian OS

Lots of inspiration drawn from the [https://github.com/remzi-arpacidusseau/ostep-projects](OSTEP xv6 projects).

Things added:

* Lottery scheduling for processes, as per OSTEP. Users can set process priorities by assigning tickets via system call.
* `getpstat` system call and corresponding `ps` user program to get summary of current process statuses.
