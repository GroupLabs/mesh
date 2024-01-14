#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include "darwin.h"

#ifdef ACCELERATE_enabled
#include "../ops/ACCELERATE_ops.h"
#endif
#ifdef MKL_enabled
#include "ops/MKL_ops.h"
#endif

#ifdef NEON_enabled // https://gist.github.com/csarron/3191b401ec545f78022881f1805cae9a
#include "ops/NEON_ops.h"
#endif

void show_cpu_info(char* buffer, size_t buffer_size)
{   
    printf("%lu", buffer_size);
    if (sysctlbyname("machdep.cpu.brand_string", buffer, &buffer_size, NULL, 0) < 0) {
        perror("sysctl");
    }
    printf("%.*s\n", (int)buffer_size, buffer);
}

// char * show_cpu_info(char* buffer)
// {
//     char buffer[1024];
//     size_t size=sizeof(buffer);
//     if (sysctlbyname("machdep.cpu.brand_string", &buffer, &size, NULL, 0) < 0) {
//         perror("sysctl");
//     }
//     printf("%d", size);
//     printf("%.*s\n", (int)size,buffer);

//     return buffer;
// }


// ACCELERATE CONFIG
// // configure known accelerate devices
// #if ACCELERATE_enabled

// int ACCELERATE_config_success = ACCELERATE_config(mesh_ptr);

// if(ACCELERATE_config_success == 0){ // ACCELERATE successfully configured
//     // 
// }
// else{ // ACCELERATE failed to configure
//     // remove anything related to ACCELERATE from mesh instance (ideally Mesh perserveres)
// }

// // configure known MKL devices
// #ifdef MKL_enabled
// //
// #endif