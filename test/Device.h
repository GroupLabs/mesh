// Device.h
#include <stdlib.h> // check size vs stddef.h
// Device manages each known device, and information on how to communicate.

#ifndef DEVICE_H
#define DEVICE_H


// All the char arrays in Device should be integer enums


// single generic device 
typedef struct {
    int device_id; // auto-assigned device_id
    int thread_id; // thread_id on device if exists, else -1
    char api_id[5]; // API ID [CUDA, MKL, etc.]

    // METADATA
    char type[8]; // Specifies the type of device [CPU, GPU, etc.]
    int rank;
} Device;



// collection of devices
typedef struct {
    int device_count; // number of devices in device_list
    
    Device *devices; // list of devices, hashmap? https://github.com/tidwall/hashmap.c

} DeviceList;

// nullifies existing object, or initalizes new DeviceList object
// Accepts: number of initial elements
// Returns: void; pointer is now pointing to intialized (null) DeviceList object
DeviceList *DeviceList_new(size_t n) {
    DeviceList *p = (DeviceList*)malloc(sizeof(DeviceList));
    if(p) {
        p->devices = (Device*)malloc(n * sizeof(Device));
        p->device_count = n;
    } else {
        printf("Memmory allocation error Device list new");
    }

    free(p);
    return p;
}

size_t resize_vector(DeviceList *v, size_t n) {
    if(v) {
        Device *p = (Device*)realloc(v->devices, n * sizeof(Device));
        if(p) {
            v->devices = p;
            v->device_count = n;
        }
        return v->device_count;
    } else {
        printf("Memmory allocation error resize vector");
    }

    return 0;
}

void set_vector(DeviceList *v, size_t n, Device x) {
    if(v) {
        if(n >= v->device_count) {
            resize_vector(v, n);
        }
        v->devices[n] = x;
    } else {
        printf("Memmory allocation error set vector");
    }
}





// multithreaded scheduler to assign tasks to different devices
// Accepts: pointer to a DeviceList struct
// Returns: success flag; 0 on success, not 0 on error
void DeviceList_scheduler()
{
    

}

#endif