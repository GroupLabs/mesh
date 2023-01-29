#include <stdio.h>
#define CUDA_EXISTS 0

// Is this more performant than using find in makefile?
// Should be as little config overhead as possible
// https://stackoverflow.com/questions/142877/can-the-c-preprocessor-be-used-to-tell-if-a-file-exists
#if __has_include("cuda.h")
#include <cuda.h>
#define CUDA_EXISTS 1
#endif


#ifdef WINDOWS
#include <Windows.h>
#else
#include <dlfcn.h>
#endif


void * loadCudaLibrary() {
#ifdef WINDOWS
    return LoadLibraryA("nvcuda.dll");
#else
    return dlopen ("libcuda.so", RTLD_NOW);
#endif
}


void (*getProcAddress(void * lib, const char *name))(void){
#ifdef WINDOWS
    return (void (*)(void)) GetProcAddress(lib, name);
#else
    return (void (*)(void)) dlsym(lib,(const char *)name);
#endif
}

int freeLibrary(void *lib)
{
#ifdef WINDOWS
    return FreeLibrary(lib);
#else
    return dlclose(lib);
#endif
}








#if CUDAEXISTS

typedef CUresult CUDAAPI (*cuInit_pt)(unsigned int Flags);
typedef CUresult CUDAAPI (*cuDeviceGetCount_pt)(int *count);
typedef CUresult CUDAAPI (*cuDeviceComputeCapability_pt)(int *major, int 
*minor, CUdevice dev);

int main() {
    void * cuLib;

    if(CUDA_EXISTS){
    cuInit_pt my_cuInit = NULL;
    cuDeviceGetCount_pt my_cuDeviceGetCount = NULL;
    cuDeviceComputeCapability_pt my_cuDeviceComputeCapability = NULL;

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

    {
        int count, i;
        if (CUDA_SUCCESS != my_cuInit(0))
            return 1; // failed to initialize
        if (CUDA_SUCCESS != my_cuDeviceGetCount(&count))
            return 1; // failed

        for (i = 0; i < count; i++)
        {
            int major, minor;
            if (CUDA_SUCCESS != my_cuDeviceComputeCapability(&major, 
&minor, i))
                return 1; // failed

            printf("dev %d CUDA compute capability major %d minor %d\n", 
i, major, minor);
        }
    }
    freeLibrary(cuLib);
    return 0;
    }
    else{
        printf("No CUDA")
    }
}

#else
int main(){
    printf("NO CUDA");
    return 0;
}

#endif