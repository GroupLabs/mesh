// Mesh.h

// Mesh provides an easy-to-use api that automatically selects a device to run on.

#ifndef MESH_H
#define MESH_H

#define NUM_INITIAL_ELEMENTS 0

#include "Device.h"


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
Mesh* new_mesh(){
    Mesh *mesh_ptr = (Mesh*)malloc(sizeof(Mesh));

    if(mesh_ptr){
        mesh_ptr->device_list = new_devicelist(NUM_INITIAL_ELEMENTS);
    } else {
        // printf("Memmory allocation error Mesh new");
    }

    return mesh_ptr;
}

// import ops for enabled apis
#include "ops/NATURAL_ops.h"
#ifdef CUDA_enabled
#include "ops/CUDA_ops.h"
#endif
#ifdef MKL_enabled
#include "ops/MKL_ops.h"
#endif

void natural_config(Mesh *mesh_ptr);

// config initializes a mesh object, and populates it with
// information about known devices and device apis
// Accepts: pointer to a Mesh struct
// Returns: void; pointer is now pointing to configured Mesh object
void config(Mesh* mesh_ptr){

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



void free_mesh(Mesh* mesh_ptr){
    free_devicelist(mesh_ptr->device_list);
}


#endif


