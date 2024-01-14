#include "../mesh.h"
#include "ACCELERATE_ops.h"

#include "../os/darwin.h"

int ACCELERATE_config(Mesh *mesh_ptr){

    Device device = {"ACCEL", "SoC", 0};

    add_device(mesh_ptr->device_list, device);

    return 0;
}