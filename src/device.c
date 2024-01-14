// Device.h
// Device manages each known device, and information on how to communicate.

#include <stdlib.h> // check size vs stddef.h
#include "device.h"

DeviceList *new_devicelist(size_t n) {
    DeviceList *p = (DeviceList*)malloc(sizeof(DeviceList));
    if(p) {
        p->devices = (Device*)malloc(n * sizeof(Device));
        p->device_count = n;
    } else {
        // printf("Memmory allocation error Device list new");
    }

    return p;
}

size_t resize_devicelist(DeviceList *v, size_t n) { // resize device list
    if(v) {
        Device *p = (Device*)realloc(v->devices, n * sizeof(Device));
        if(p) {
            v->devices = p;
            v->device_count = n;
        }
        return v->device_count;
    } else {
        // printf("Memmory allocation error resize vector");
    }

    return 0;
}

void set_device(DeviceList *v, Device x, size_t index) { // set device at index
    if(v) {
        if(index >= v->device_count) {
            resize_devicelist(v, index);
        }
        v->devices[index] = x;
    } else {
        // printf("Memmory allocation error set vector");
    }
}

void add_device(DeviceList *v, Device x) { // add device to end of list
    if(v) {

        resize_devicelist(v, v->device_count + 1);

        x.device_id = v->device_count - 1;
        x.thread_id = -1;

        v->devices[v->device_count - 1] = x;
    } else {
        // printf("Memmory allocation error set vector");
    }
}

void free_devicelist(DeviceList *v) { // free device list
    if(v) {
        free(v->devices);
        free(v);
    }
}

void devicelist_scheduler()
{
    

}