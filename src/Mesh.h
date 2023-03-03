// Mesh.h

// Mesh provides an easy-to-use api that automatically selects a device to run on.

#ifndef MESH_H
#define MESH_H

#include "Device.h"

#define NUM_INITIAL_ELEMENTS 0

typedef struct {
    // int available_apis[3]; // Should be some other DS - 2D char array requires strncopy, hashmap?
    // For now, 
    // -1 - Natural (Device this runs on initially)
    // 0 - Unassigned
    // 1 - CUDA
    // 2 - OneAPI


    DeviceList* device_list;
} Mesh;


// initalizes new Mesh object
// Accepts: void
// Returns: pointer to a Mesh struct
Mesh* new_mesh();

// config initializes a mesh object, and populates it with
// information about known devices and device apis
// Accepts: pointer to a Mesh struct
// Returns: void; pointer is now pointing to configured Mesh object
void config(Mesh* mesh_ptr);

void free_mesh(Mesh* mesh_ptr);

#endif


