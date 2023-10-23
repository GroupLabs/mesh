// Mesh.h

// Mesh provides an easy-to-use api that automatically selects a device to run on.
#include "mesh.h"
#include "device.h"

#ifdef UNKNOWN_PROC
#include "ops/NATURAL_ops.h"
#endif

#ifdef ACCELERATE_enabled
#include "ops/ACCELERATE_ops.h"
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

    #ifdef ACCELERATE_enabled

    ACCELERATE_config(mesh_ptr);

    #endif

    // for each system, build a seperate thread, trigger appropriate os config 
}

void free_mesh(Mesh* mesh_ptr){
    free_devicelist(mesh_ptr->device_list);
}


