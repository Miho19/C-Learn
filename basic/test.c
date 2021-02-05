#include <stdlib.h>
#include <stdio.h>

#include "input.h"
#include <string.h>

device USB[MAX_DEVICE];

void dhandler_init(){
    memset(USB, 0, sizeof(USB[0]) * MAX_DEVICE);
}




