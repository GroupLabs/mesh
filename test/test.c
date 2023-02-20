#include <stdio.h>
#include "Mesh.h"

// Only exists to debug vals
int main(){

    struct Mesh mesh;

    config(&mesh);

    printf("\nBuilt mesh object");

    printf("\n\nDevice count: %d\n\n", mesh.device_list->device_count);

    // for(int i = 0; i < 3; i++){

    //     printf("Device ID: %ld\n", mesh.device_list.device_map[i].device_id);
    //     printf("Active thread ID: %ld\n", mesh.device_list.device_map[i].thread_id);
    //     printf("API ID: %s\n", mesh.device_list.device_map[i].api_id);

    //     printf("Type: %s\n\n", mesh.device_list.device_map[i].type);
    // }

}