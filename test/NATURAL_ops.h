// NATURAL_ops.h
#ifndef NATURAL_OPS_H
#define NATURAL_OPS_H

#include "Mesh.h"
#include "String_H.h"

void natural_config(struct Mesh *mesh_ptr){
    // Set natural API
    mesh_ptr->available_apis[0] = -1; // Should find available slot, then assign

    // Add device to device_list
    Device natural_device = {"NTRL", "CPU", 0}; // Literally just guessing it's CPU

    #ifdef AMD64
        strlcpy(natural_device.api_id, "AMD64", API_ID_LEN);
    #endif
    #ifdef IA32
        strlcpy(natural_device.api_id, "IA32", API_ID_LEN);
    #endif
    #ifdef ARM
        strlcpy(natural_device.api_id, "ARM", API_ID_LEN);
    #endif

    add_device(mesh_ptr->device_list, natural_device);
    add_device(mesh_ptr->device_list, natural_device);
    add_device(mesh_ptr->device_list, natural_device);
}

#endif