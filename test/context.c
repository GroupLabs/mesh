// context.c
#include "Device.h"

// import ops
#ifdef CUDA_enabled
#include "CUDA_ops.h"
#endif
#ifdef MKL_enabled
#include "MKL_ops.h"
#endif


// Tensor?

struct{

    // dimension queue
    // queue dimensions


};


// 
struct Mesh{

    struct Device_List device_list;



};


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


int main(){

    // Take in arguments to see what headers are enabled






    printf("CONTEXT INVOKED");
    printf("OS, CPU, ARCH, CUDA-ENABLED");
    return 0;
}