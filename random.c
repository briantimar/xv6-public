#include "types.h"
#include <stdio.h>


static uint seed = 1;
static uint a = 1664525;
static uint c = 1013904223;
static uint m = 0xffffffff;

// sample a random integer in the range 0..< 2^32
// using LCG params from https://en.wikipedia.org/wiki/Linear_congruential_generator

// TODO using the value of the modulus leads to undefined behavior!
uint
rand(void)
{
    seed = ((a * seed) + c) %m;
    return seed;
}

// TODO cache the weight values, these are almost entirely static
// TODO you can binary-search for the correct bin if you store the partial sums.
// given array of positive integer weights w, return index i with probability w[i]
int sampleindex(int w[], int numweights) {
    int sum=0;
    int binupper, binlower;
    int i;
    if (numweights <= 0) {
        return -1;
    }
    // get sum of the weights array
    for (i=0; i < numweights; i++) {
        sum += w[i];
    }
    // drop sample into corresponding bins
    int sample = (int) (rand() % sum);
    // return bin into which sample has fallen
    binupper=0;
    for (i=0; i< numweights; i++) {
        binlower = binupper;
        binupper = binlower + w[i];
        if ((binlower <= sample) && (sample < binupper)) {
            return i;
        }
    }
    return -1;
}

