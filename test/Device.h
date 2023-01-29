// Device.h

#ifndef DEVICE_H
#define DEVICE_H

#include <stdio.h>

// Single device 
struct Device {
    long int device_id; // auto-assigned device_id
    long int thread_id; // thread_id on device if exists, else -1
    char api_id[5]; // API ID [CUDA, MKL, etc.]

    // METADATA
    char type[8]; // Specifies the type of device [CPU, GPU, etc.]
};

// Just a thought
// #ifdef CUDA_enabled
// // CUDA Device
// struct CUDA_Device {
//     long int device_id; // auto-assigned device_id
//     long int thread_id; // thread_id on device if exists, else -1
//     char api_id[5]; // API ID [CUDA, MKL, etc.]

//     // METADATA
//     char type[8]; // Specifies the type of device [CPU, GPU, etc.]
// };
// #endif

// hashmap of devices (device_list)
struct DeviceList {
    int device_count; // number of devices in device_list
    
    struct Device device_map[5]; // list of devices, hashmap? https://github.com/tidwall/hashmap.c
};

void DeviceList_new(struct DeviceList* device_list){
    device_list->device_count = 0;
    // device_list->device_list = ; // should invoke destroy() or free() then NULL
}

#endif