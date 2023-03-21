// Device.h
// Device manages each known device, and information on how to communicate.

#ifndef DEVICE_H
#define DEVICE_H

#include <stdlib.h>

#define API_ID_LEN 6
#define TYPE_ID_LEN 5

// single generic device 
typedef struct {
    char api_id[API_ID_LEN]; // API ID [CUDA, MKL, etc.]

    char type[TYPE_ID_LEN]; // Specifies the type of device [CPU, GPU, etc.]
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
DeviceList *new_devicelist(size_t n);

size_t resize_devicelist(DeviceList *v, size_t n);

void set_device(DeviceList *v, Device x, size_t index);

void add_device(DeviceList *v, Device x);

void free_devicelist(DeviceList *v);


// multithreaded scheduler to assign tasks to different devices
// Accepts: pointer to a DeviceList struct
// Returns: success flag; 0 on success, not 0 on error
void devicelist_scheduler();

#endif