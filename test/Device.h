// Device.h

// Single device 
struct Device {
    long int device_id; // auto-assigned device_id
    long int thread_id; // thread_id on device if exists, else -1
    char api_id[5]; // API ID [CUDA, MKL, etc.]

    // METADATA
    char type[8]; // Specifies the type of device [CPU, GPU, etc.]
};

// hashmap of devices (device_list)
struct Device_List {
    long int device_count; // number of devices in device_list
    
    // device_list; // list of devices
};

// config
// Takes in pointer to device_list, list of enabled APIs
// populates it with all devices
void config(){
    // get all devices in device_list
}