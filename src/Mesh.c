// Mesh.h

// Mesh provides an easy-to-use api that automatically selects a device to run on.
#include "Mesh.h"
#include "Device.h"

// import ops for enabled apis
#ifdef UNKNOWN_PROC
#include "ops/NATURAL_ops.h"
#endif
#ifdef ACCELERATE_enabled
#include "ops/ACCELERATE_ops.h"
#endif
#ifdef CUDA_enabled
#include "ops/CUDA_ops.h"
#endif
#ifdef MKL_enabled
#include "ops/MKL_ops.h"
#endif
#ifdef NEON_enabled // https://gist.github.com/csarron/3191b401ec545f78022881f1805cae9a
#include "ops/NEON_ops.h"
#endif

Mesh* new_mesh(){
    Mesh *mesh_ptr = (Mesh*)malloc(sizeof(Mesh));

    if(mesh_ptr){
        mesh_ptr->device_list = new_devicelist(NUM_INITIAL_ELEMENTS);
    } else {
        // printf("Memmory allocation error Mesh new");
    }

    return mesh_ptr;
}

void config(Mesh* mesh_ptr){

    #ifdef UNKNOWN_PROC

    // get naturally available devices
    natural_config(mesh_ptr);

    #endif
    

    // configure known accelerate devices
    #if ACCELERATE_enabled

    int ACCELERATE_config_success = ACCELERATE_config(mesh_ptr);

    if(ACCELERATE_config_success == 0){ // ACCELERATE successfully configured
        // 
    }
    else{ // ACCELERATE failed to configure
        // remove anything related to ACCELERATE from mesh instance (ideally Mesh perserveres)
    }

    #endif

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


