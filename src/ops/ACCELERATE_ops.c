#include "../mesh.h"
#include "ACCELERATE_ops.h"

#include "../os/darwin.h"

int ACCELERATE_config(Mesh *mesh_ptr){

    char *device_type = (char*) malloc(1024*sizeof(char));
    char *api_id = (char*) malloc(1024*sizeof(char));

    free(device_type);

    Device device = {"ACCEL", "SoC", 0};

    size_t a = 1024;

    show_cpu_info(device_type, a);

    add_device(mesh_ptr->device_list, device);

    return 0;
}