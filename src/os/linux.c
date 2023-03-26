// #ifdef CUDA_enabled
// #include "ops/CUDA_ops.h"
// #endif
// #ifdef MKL_enabled
// #include "ops/MKL_ops.h"
// #endif


// configure known CUDA devices
// #if CUDA_enabled

// int CUDA_config_success = CUDA_config(mesh_ptr);

// if(CUDA_config_success == 0){ // CUDA successfully configured
//     // 
// }
// else{ // CUDA failed to configure
//     // remove anything related to CUDA from mesh instance (ideally Mesh perserveres)
// }

// // configure known MKL devices
// #ifdef MKL_enabled
// //
// #endif