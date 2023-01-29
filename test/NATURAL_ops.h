// NATURAL_ops.h

#ifndef NATURAL_OPS_H
#define NATURAL_OPS_H

#include "Mesh.h"

void natural_config(struct Mesh *mesh_ptr){
    // Set natural API
    mesh_ptr->available_apis[0] = -1; // Should find available slot, then assign

    // Increment device_count
    mesh_ptr->device_list.device_count += 1;

    // Add device to device_list
    struct Device natural_device = { 0, -1, "NTRL", "CPU"}; // Literally just guessing it's CPU
    mesh_ptr->device_list.device_map[0] = natural_device; // should not assign to hardcoded index
}

#endif