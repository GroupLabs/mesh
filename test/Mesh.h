// Mesh.h
#ifndef MESH_H
#define MESH_H

#include "Device.h"

struct Mesh{
    int available_apis[3]; // Should be some other DS - 2D char array requires strncopy, hashmap?
    // For now, 
    // -1 - Natural (Device this runs on initially)
    // 0 - Unassigned
    // 1 - CUDA
    // 2 - OneAPI


    struct DeviceList device_list;
};

void Mesh_new(struct Mesh * mesh_ptr){

    // mesh_ptr->available_apis[0] // might be initialized differently if we change DS

    DeviceList_new(&(mesh_ptr->device_list));
}



#endif