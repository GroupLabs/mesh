#include <stdio.h>
#include <stdlib.h>
#include <sys/sysctl.h>
#include <IOKit/graphics/IOGraphicsLib.h>
#include <CoreFoundation/CoreFoundation.h>

void print_cpu_info() {
    int ncpu;
    size_t ncpu_len = sizeof(ncpu);
    if (sysctlbyname("hw.logicalcpu", &ncpu, &ncpu_len, NULL, 0) == -1) {
        perror("sysctlbyname");
        return;
    }

    printf("Available CPUs: %d\n", ncpu);
}

void print_gpu_info() {
    CFMutableDictionaryRef matching, properties = NULL;
    io_iterator_t iterator;
    io_registry_entry_t device;

    matching = IOServiceMatching("IOPCIDevice");
    IOServiceGetMatchingServices(kIOMasterPortDefault, matching, 
&iterator);

    while ((device = IOIteratorNext(iterator))) {
        CFMutableDictionaryRef service_properties;
        if (IORegistryEntryCreateCFProperties(device, &service_properties, 
kCFAllocatorDefault, kNilOptions) == KERN_SUCCESS) {
            CFDataRef model = 
(CFDataRef)CFDictionaryGetValue(service_properties, CFSTR("model"));
            if (model) {
                char model_buffer[256];
                snprintf(model_buffer, sizeof(model_buffer), "%.*s", 
(int)CFDataGetLength(model), CFDataGetBytePtr(model));
                printf("GPU: %s\n", model_buffer);
            }
            CFRelease(service_properties);
        }
        IOObjectRelease(device);
    }
    IOObjectRelease(iterator);
}

int main() {
    print_cpu_info();
//    print_gpu_info();

    return 0;
}

