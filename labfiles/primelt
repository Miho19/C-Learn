#!/usr/bin/env python

import sys, math

def factors(x) :
    f = 2
    while f <= math.sqrt(x) :
        if x % f == 0 :
            return 0
        else :
            f = f + 1
    return 1

bound = int(sys.argv[1])-1
while bound > 0 :
    if factors(bound) :
        print bound
        break
    else :
        bound = bound - 1
        
