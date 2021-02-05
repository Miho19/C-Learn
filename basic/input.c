#include <stdlib.h>
#include <stdio.h>

#include "input.h"

int main() {
    device new_device;
    int i;
    
    
    dhandler_init();
    new_device = malloc(sizeof(device));
    new_device->major = 15;
    new_device->usb_active = 0;
    USB[0] = new_device;

    USB[0]->usb_active = 1;
    printf("usb_active = %d\n", USB[0]->usb_active);

    for(i = 0;i < MAX_DEVICE;i++)
        free(USB[i]);
    return 0;
}