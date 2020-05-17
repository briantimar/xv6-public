#include "types.h"
#include <stdio.h>

// sample a random integer in the range 0..< 2^32
// using LCG params from https://en.wikipedia.org/wiki/Linear_congruential_generator

static uint seed = 0;
static uint a = 1664525;
static uint c = 1013904223;
static uint m = 0xffffffff;

uint
rand(void)
{
    seed = ((a * seed) + c) %m;
    return seed;
}
