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

#include <stdio.h>
#include <string.h>

const int SKIPCHAR = 2;

//More efficient to run this than run lscpu?
void show_cpu_info(char* buffer, size_t buffer_size) {
	printf("%lu", buffer_size);
	FILE * file = fopen("/proc/cpuinfo", "r");
	if (file == NULL) {
		perror("fopen");
		return;
	}

	while (fgets(buffer, buffer_size, file) != NULL) {
		if (strncmp(buffer, "model name", 10) == 0) {
			char* brand_string = strchr(buffer, ':') + SKIPCHAR;
			snprintf(buffer, buffer_size, "%s", brand_string);
			break;
		}
	}

	fclose(file);
	printf("%s", buffer);
}
