// Device.h
// Device manages each known device, and information on how to communicate.

#ifndef DEVICE_H
#define DEVICE_H

#include <stdlib.h>

#define API_ID_LEN 6
#define TYPE_ID_LEN 5

enum op {
    NONE = 0,

    // unary
    EXP,
    LOG,
    CAST,

    // binary
    ADD,
    SUB,
    MUL,
    DIV,
    POW,
    CMPEQ,
    MAX,

    // reduction
    SUM,
    R_MAX,

    // movement
    EXPAND,
    RESHAPE,
    PERMUTE,
    PAD,
    SHRINK,
    STRIDE

    // DUP,
    // SQR,
    // SQRT,
    // SUM,
    // MEAN,
    // REPEAT,
    // ABS,
    // SGN,
    // NEG,
    // STEP,
    // RELU,
    // GELU,
    // SILU,
    // NORM, // normalize
    // RMS_NORM,

    // MUL_MAT,

    // SCALE,
    // CPY,
    // RESHAPE,
    // VIEW,
    // PERMUTE,
    // TRANSPOSE,
    // GET_ROWS,
    // DIAG_MASK_INF,
    // SOFT_MAX,
    // ROPE,
    // CONV_1D_1S,
    // CONV_1D_2S,

    // FLASH_ATTN,
    // FLASH_FF,

    // COUNT,
};

// single generic device - this is a fuzzy concept; devices are engaged implicitly
// by the scheduler, to not override other mechanisms
typedef struct {
    int device_id; // auto-assigned device_id
    int thread_id; // thread_id on device if exists, else -1 (handled by scheduler)

    // operations

    // tensor (*none)(tensor, tensor); // foo = &device_specific_func;

    // // unary
    // tensor (*exp)(tensor, tensor);
    // tensor (*log)(tensor, tensor);
    // tensor (*cast)(tensor, tensor);

    // // binary
    // tensor (*add)(tensor, tensor); 
    // tensor (*sub)(tensor, tensor);
    // tensor (*mul)(tensor, tensor);
    // tensor (*div)(tensor, tensor);
    // tensor (*pow)(tensor, tensor);
    // tensor (*cmpeq)(tensor, tensor);
    // tensor (*max)(tensor, tensor);

    // // reduction
    // tensor (*sum)(tensor, tensor);
    // tensor (*r_max)(tensor, tensor);

    // // movement
    // tensor (*expand)(tensor, tensor);
    // tensor (*reshape)(tensor, tensor);
    // tensor (*premute)(tensor, tensor);
    // tensor (*pad)(tensor, tensor);
    // tensor (*shrink)(tensor, tensor);
    // tensor (*stride)(tensor, tensor);

} Device;

// collection of devices
typedef struct {
    int device_count; // number of devices in device_list
    
    Device *devices; // list of devices, hashmap? https://github.com/tidwall/hashmap.c

} DeviceList;

// nullifies existing object, or initalizes new DeviceList object
// Accepts: number of initial elements
// Returns: void; pointer is now pointing to intialized (null) DeviceList object
DeviceList *new_devicelist(size_t n);

size_t resize_devicelist(DeviceList *v, size_t n);

void set_device(DeviceList *v, Device x, size_t index);

void add_device(DeviceList *v, Device x);

void free_devicelist(DeviceList *v);


// multithreaded scheduler to assign tasks to different devices
// Accepts: pointer to a DeviceList struct
// Returns: success flag; 0 on success, not 0 on error
void devicelist_scheduler();

#endif