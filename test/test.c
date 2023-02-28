#include <stdio.h>
#include "Mesh.h"

// Only exists to debug vals
int main(){

    struct Mesh mesh;

    config(&mesh);

    printf("\nBuilt mesh object");

    printf("\n\nDevice count: %d\n\n", mesh.device_list->device_count);

    int device_count = mesh.device_list->device_count;

    for(int i = 0; i < device_count; i++){

        printf("Device ID: %d\n", mesh.device_list->devices[i].device_id);
        printf("Active thread ID: %d\n", mesh.device_list->devices[i].thread_id);
        printf("API ID: %s\n", mesh.device_list->devices[i].api_id);
        printf("Type: %s\n\n", mesh.device_list->devices[i].type);
    }

}