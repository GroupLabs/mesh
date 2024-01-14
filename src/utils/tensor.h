#ifndef DEVICE_H
#define DEVICE_H

#define MAX_DIMS 4

typedef struct {
    int n_dims;

    // op _op; // op that created this tensor

    struct tensor * grad;
    struct tensor * src0;
    struct tensor * src1;

    void * data;
} tensor;



#endif