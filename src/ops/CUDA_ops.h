// CUDA_ops.h

#if CUDA_enabled

#ifndef CUDA_OPS_H
#define CUDA_OPS_H

#include "../Mesh.h"

// Is this more performant than using find in makefile?
// Should be as little config overhead as possible
// https://stackoverflow.com/questions/142877/can-the-c-preprocessor-be-used-to-tell-if-a-file-exists
// #if __has_include("cuda.h")
#include <cuda.h>
// #define CUDA_EXISTS 1
// #endif

// Check if CUDA is available
// https://stackoverflow.com/questions/15860390/program-to-check-cuda-presence-needs-cuda

// Enables dynamic linking
#ifdef WINDOWS
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

// Loads CUDA library
void * loadCudaLibrary() {
    #ifdef WINDOWS
        return LoadLibraryA("nvcuda.dll");
    #else
        return dlopen("libcuda.so", RTLD_NOW);
    #endif
    // Should this try for cudart too?
}

// 
void (*getProcAddress(void * lib, const char *name))(void){
    #ifdef WINDOWS
        return (void (*)(void)) GetProcAddress(lib, name);
    #else
        return (void (*)(void)) dlsym(lib,(const char *)name);
    #endif
}

// Frees dl library
int freeLibrary(void *lib){
    #ifdef WINDOWS
    return FreeLibrary(lib);
    #else
    return dlclose(lib);
    #endif
}

typedef CUresult CUDAAPI (*cuInit_pt)(unsigned int Flags);
typedef CUresult CUDAAPI (*cuDeviceGetCount_pt)(int *count);
typedef CUresult CUDAAPI (*cuDeviceComputeCapability_pt)(int *major, int 
*minor, CUdevice dev);


int CUDA_config(struct Mesh* mesh_ptr){

    void * cuLib;
    cuInit_pt my_cuInit = NULL;
    cuDeviceGetCount_pt my_cuDeviceGetCount = NULL;
    cuDeviceComputeCapability_pt my_cuDeviceComputeCapability = NULL;

    // Checks if library loads properly (Should use dlerror instead?)
    {
        if ((cuLib = loadCudaLibrary()) == NULL)
            return 1; // cuda library is not present in the system

        if ((my_cuInit = (cuInit_pt) getProcAddress(cuLib, "cuInit")) == NULL)
            return 1; // sth is wrong with the library
        
        if ((my_cuDeviceGetCount = (cuDeviceGetCount_pt) getProcAddress(cuLib, 
        "cuDeviceGetCount")) == NULL)
            return 1; // sth is wrong with the library

        if ((my_cuDeviceComputeCapability = (cuDeviceComputeCapability_pt) 
    getProcAddress(cuLib, "cuDeviceComputeCapability")) == NULL)
            return 1; // sth is wrong with the library
    }

    // Appends device objects to device_list
    {
        int count, i;
        if (CUDA_SUCCESS != my_cuInit(0))
            return 1; // failed to initialize
        if (CUDA_SUCCESS != my_cuDeviceGetCount(&count))
            return 1; // failed

        for (i = 0; i < count; i++)
        {
            mesh_ptr->available_apis[i+1] = 1; // Should find available slot, then assign (this just adds after first index REELLY BAD)
            // Also should only add if doesn't exist HASHMAP??

            // Add device to device_list
            Device CUDA_device = {"CUDA", "GPU", 0}; // Literally just guessing it's GPU
            
            add_device(mesh_ptr->device_list, CUDA_device); // should not assign to hardcoded index

            int major, minor;
            if (CUDA_SUCCESS != my_cuDeviceComputeCapability(&major, &minor, i))
                return 1; // failed

            printf("\ndev %d CUDA compute capability major %d minor %d\n", i, major, minor); // To illustrate that we aren't getting these vals
        }
    }



    freeLibrary(cuLib); // Should this be here?? or in freeMesh??
    return 0;
}















#endif

#endif