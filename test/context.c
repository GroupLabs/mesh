// context.c
#include "Device.h"
#include "Mesh.h"

// DEBUG
#define DEBUG 1

#if DEBUG
#include <stdio.h>
#endif

// import ops for enabled apis
#include "NATURAL_ops.h"
#ifdef CUDA_enabled
#include "CUDA_ops.h"
#endif
#ifdef MKL_enabled
#include "MKL_ops.h"
#endif

// config
// Takes in pointer to mesh
// initializes it with
// 1. Device List
// 2. etc
void config(struct Mesh* mesh_ptr){

    // initialize mesh object with null values
    Mesh_new(mesh_ptr);

    // get naturally available devices
    natural_config(mesh_ptr);

    // get all devices in device_list
    #if CUDA_enabled

    
    printf("\nCUDA is enabled\n");
    // Takes in ptr to device list, 
    // appends CUDA devices to device_list
    // returns success flag
    int CUDA_config_success = CUDA_config(mesh_ptr);

    if(CUDA_config_success == 1){
        // remove anything related to CUDA from mesh instance
        
    }

    printf("CUDA success flag: %d\n", CUDA_config_success);

    #endif

    #ifdef MKL_enabled
    //
    #endif

    // preprocessor flags mean nothing at this point. All Macro states are no longer representative.
}













// op: multiply
// takes in pointer to Mesh obj, tensorA, tensorB
// places operation on viable device
void multiply(){

// use scheduler to find device
// put onto that device

}

// op: add


// scheduler
// find device
void scheduler()
{
    

}


#if DEBUG

// Only exists to debug vals
int main(){

    struct Mesh mesh;

    config(&mesh);

    printf("\nBuilt mesh object");

    printf("\n\nDevice count: %d\n\n\n", mesh.device_list.device_count);

    for(int i = 0; i < 3; i++){

        printf("Device ID: %ld\n", mesh.device_list.device_map[i].device_id);
        printf("Active thread ID: %ld\n", mesh.device_list.device_map[i].thread_id);
        printf("API ID: %s\n", mesh.device_list.device_map[i].api_id);

        printf("Type: %s\n\n", mesh.device_list.device_map[i].type);
    }

}

#endif