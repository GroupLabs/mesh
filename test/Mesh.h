// Mesh.h

// Mesh provides an easy-to-use api that automatically selects a device to run on.

#ifndef MESH_H
#define MESH_H

#define NUM_INITIAL_ELEMENTS 1

#include "Device.h"

struct Mesh{
    int available_apis[3]; // Should be some other DS - 2D char array requires strncopy, hashmap?
    // For now, 
    // -1 - Natural (Device this runs on initially)
    // 0 - Unassigned
    // 1 - CUDA
    // 2 - OneAPI


    DeviceList* device_list;
};

// nullifies existing object, or initalizes new Mesh object
// Accepts: pointer to a Mesh struct
// Returns: void; pointer is now pointing to intialized (null) Mesh object
void Mesh_new(struct Mesh * mesh_ptr){

    // mesh_ptr->available_apis[0] // might be initialized differently if we change DS

    mesh_ptr->device_list = DeviceList_new(NUM_INITIAL_ELEMENTS);
}

// import ops for enabled apis
#include "NATURAL_ops.h"
#ifdef CUDA_enabled
#include "CUDA_ops.h"
#endif
#ifdef MKL_enabled
#include "MKL_ops.h"
#endif

// config initializes a mesh object, and populates it with
// information about known devices and device apis
// Accepts: pointer to a Mesh struct
// Returns: void; pointer is now pointing to configured Mesh object
void config(struct Mesh* mesh_ptr){

    // initialize mesh object with null values
    Mesh_new(mesh_ptr);

    // get naturally available devices
    natural_config(mesh_ptr);

    // configure known CUDA devices
    #if CUDA_enabled

    int CUDA_config_success = CUDA_config(mesh_ptr);

    if(CUDA_config_success == 0){ // CUDA successfully configured
        // 
    }
    else{ // CUDA failed to configure
        // remove anything related to CUDA from mesh instance (ideally Mesh perserveres)
    }

    #endif

    // configure known MKL devices
    #ifdef MKL_enabled
    //
    #endif

    // note: since device initialization could fail, flags provided at compile time may not be
    // representative at run time.
}






#endif


