### tests for the kernel

How testing works, in broad strokes:

1. Place C code in here which prints diagnostics to stdout
2. Copy test code to user program list, build fs img and boot
3. run all test scripts, use expect script to capture output to local text file
4. compare against expected output.

I'm borrowing heavily here from the testing framework developed for [ostep-projects](https://github.com/remzi-arpacidusseau/ostep-projects).