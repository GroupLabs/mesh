// Device.h

// Device manages each known device, and information on how to communicate.

#ifndef DEVICE_H
#define DEVICE_H

// single generic device 
struct Device {
    long int device_id; // auto-assigned device_id
    long int thread_id; // thread_id on device if exists, else -1
    char api_id[5]; // API ID [CUDA, MKL, etc.]

    // METADATA
    char type[8]; // Specifies the type of device [CPU, GPU, etc.]
    int rank;
};

// collection of devices
struct DeviceList {
    int device_count; // number of devices in device_list
    
    struct Device device_map[5]; // list of devices, hashmap? https://github.com/tidwall/hashmap.c
};

// nullifies existing object, or initalizes new DeviceList object
// Accepts: pointer to a DeviceList struct
// Returns: void; pointer is now pointing to intialized (null) DeviceList object
void DeviceList_new(struct DeviceList* device_list){
    device_list->device_count = 0;
    // device_list->device_list = ; // should invoke destroy() or free() then NULL
}

// multithreaded scheduler to assign tasks to different devices
// Accepts: pointer to a DeviceList struct
// Returns: success flag; 0 on success, not 0 on error
void DeviceList_scheduler()
{
    

}

#endif