// NATURAL_ops.h

#ifndef NATURAL_OPS_H
#define NATURAL_OPS_H

#include "Mesh.h"

    int device_id; // auto-assigned device_id
    int thread_id; // thread_id on device if exists, else -1
    char api_id[5]; // API ID [CUDA, MKL, etc.]

    // METADATA
    char type[8]; // Specifies the type of device [CPU, GPU, etc.]
    int rank;

void natural_config(struct Mesh *mesh_ptr){
    // Set natural API
    mesh_ptr->available_apis[0] = -1; // Should find available slot, then assign

    // Add device to device_list
    Device natural_device = { 0, -1, "NTRL", "CPU", 0}; // Literally just guessing it's CPU

    set_vector(*(mesh_ptr->device_list.devices), 1, natural_device); // should not assign to hardcoded index
}

#endif