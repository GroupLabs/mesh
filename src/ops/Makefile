#include <stdlib.h>
#include <stdio.h>
#include "../Mesh.h"

#ifdef WINDOWS
    #include <Windows.h>
#else
    #include <dlfcn.h>
#endif


int oneAPIExists() {
    int result = system("which dpcpp > /dev/null 2>&1");
    if (result == 0) {
        printf("oneAPI is installed!\n");
        return 1;
    } else {
        result = system("which intel-oneapi-compilers > /dev/null 2>&1");
        if (result == 0) {
            printf("oneAPI is installed!\n");
            return 1;
        } else {
            printf("oneAPI is not installed.\n");
            return -1;
        }
    }
    return 0;
}

void* loadLibrary() {
    #ifdef WINDOWS
        HINSTANCE setup = LoadLibrary("oneapi.dll");
        
        if(setup == NULL) {
            int errnum = GetLastError();
            printf("Failed to setup oneAPI Library!\n");
            printf("System: Windows\n");
            printf("Errno: %d\n", errnum);
            return NULL;
        } else {
            printf("Successfully loaded oneAPI Library!\n")
        }
        return setup;
    #else
        void* setup = dlopen("liboneapi.so", RTLD_NOW);
        if(setup == NULL) {
            printf("Failed to setup oneAPI Library!\n");
            #if defined(__APPLE__)
                printf("System: macOS\n");
            #elif defined(__linux__)
                printf("System: Linux\n");
            #else
                printf("System: UNKNOWN\n");
            #endif
            printf("Dlerror: %s\n", dlerror());
            return NULL;
        } else {
            return setup;
        }
    #endif
}

int freeLibrary(void* lib) {
    #ifdef WINDOWS
        return FreeLibrary(lib);
    #else
        return dlclose(lib);
    #endif
}

int main() {
    int exists = oneAPIExists();
    if(exists == 1) {
        void* lib = loadLibrary();
        if (lib != NULL) {
            // use the library here
            freeLibrary(lib);
        } else {
            return -1;
        }
    } else if(exists == -1) {
        return -1;
    } else {
        return 0;
    }
    return 0;
}
