#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include "darwin.h"

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
