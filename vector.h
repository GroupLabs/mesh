#include <stdlib.h> // check size vs stddef.h

typedef struct {
    int device_id; // auto-assigned device_id
    int thread_id; // thread_id on device if exists, else -1
    char api_id[5]; // API ID [CUDA, MKL, etc.]

    // METADATA
    char type[8]; // Specifies the type of device [CPU, GPU, etc.]
    int rank;
} Device;

typedef struct {
    int device_count; // number of devices in device_list
    
    Device *devices; // list of devices, hashmap? https://github.com/tidwall/hashmap.c

} DeviceList;

DeviceList *create_vector(size_t n) {
    DeviceList *p = (DeviceList*)malloc(sizeof(DeviceList));
    if(p) {
        p->devices = (Device*)malloc(n * sizeof(Device));
        p->device_count = n;
    }
    return p;
}

void delete_vector(DeviceList *v) { // Memory leak ??
    if(v) {
        free(v->devices);
        free(v);
    }
}

size_t resize_vector(DeviceList *v, size_t n) {
    if(v) {
        Device *p = (Device*)realloc(v->devices, n * sizeof(Device));
        if(p) {
            v->devices = p;
            v->device_count = n;
        }
        return v->device_count;
    }
    return 0;
}

Device get_vector(DeviceList *v, size_t n) {
    if(v && n < v->device_count) {
        return v->devices[n];
    }
    /* return some error value, i'm doing -1 here, 
     * std::vector would throw an exception if using at() 
     * or have UB if using [] */

    return;
}

void set_vector(DeviceList *v, size_t n, Device x) {
    if(v) {
        if(n >= v->device_count) {
            resize_vector(v, n);
        }
        v->devices[n] = x;
    }
}