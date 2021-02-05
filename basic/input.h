void dhandler_init();

struct USB_device{
    short major;
    int usb_active;
};

typedef struct USB_device *device;
#define MAX_DEVICE 10

extern device USB[MAX_DEVICE];