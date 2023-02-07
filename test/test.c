#include <stdio.h>
#include "Mesh.h"

struct testA{
    int a;
    int b;
};

struct testB{
    long int c;
    int d;
};

// Only exists to debug vals
int main(){

//     struct testA G = {1, 2};

//     struct testB F = {3, 4};

//     void * ptrA = &G;

//     struct testB * ptrB = &F;

//    void * ptrArray[2] = {&G, &F};

//    printf("%ld", G.a);


    struct Mesh mesh;

    config(&mesh);

    printf("\nBuilt mesh object");

    printf("\n\nDevice count: %d\n\n", mesh.device_list.device_count);

    for(int i = 0; i < 3; i++){

        printf("Device ID: %ld\n", mesh.device_list.device_map[i].device_id);
        printf("Active thread ID: %ld\n", mesh.device_list.device_map[i].thread_id);
        printf("API ID: %s\n", mesh.device_list.device_map[i].api_id);

        printf("Type: %s\n\n", mesh.device_list.device_map[i].type);
    }

}