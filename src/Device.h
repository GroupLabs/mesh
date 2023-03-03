// Device.h
// Device manages each known device, and information on how to communicate.

#ifndef DEVICE_H
#define DEVICE_H

#include <stdlib.h> // check size vs stddef.h

#define API_ID_LEN 7

// single generic device 
typedef struct {
    char api_id[API_ID_LEN]; // API ID [CUDA, MKL, etc.]

    char type[8]; // Specifies the type of device [CPU, GPU, etc.]
    int rank; // rank of device

    int device_id; // auto-assigned device_id
    int thread_id; // thread_id on device if exists, else -1
} Device;

// collection of devices
typedef struct {
    int device_count; // number of devices in device_list
    
    Device *devices; // list of devices, hashmap? https://github.com/tidwall/hashmap.c

} DeviceList;

// nullifies existing object, or initalizes new DeviceList object
// Accepts: number of initial elements
// Returns: void; pointer is now pointing to intialized (null) DeviceList object
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


// multithreaded scheduler to assign tasks to different devices
// Accepts: pointer to a DeviceList struct
// Returns: success flag; 0 on success, not 0 on error
void devicelist_scheduler()
{
    

}

#endif