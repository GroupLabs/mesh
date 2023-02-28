// NATURAL_ops.h

#ifndef NATURAL_OPS_H
#define NATURAL_OPS_H

#include "Mesh.h"

void natural_config(struct Mesh *mesh_ptr){
    // Set natural API
    mesh_ptr->available_apis[0] = -1; // Should find available slot, then assign

    char CPU_desc[5];

    // Add device to device_list
    Device natural_device = {"NTRL", "CPU", 0}; // Literally just guessing it's CPU

    add_vector(mesh_ptr->device_list, natural_device);
}

#endif