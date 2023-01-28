# Builds mesh based on OS, and ARCH

# https://github.com/wolfSSL/wolfssl-examples/blob/master/tls/Makefile




# ALL configs should happen in makefile to keep binaries as small as possible!
# Objectives:
# Find arch, os
# Find cuda-capability
# Use cmd line flags to set Macros to inform context
# Conditionally add libraries that provide Mesh support for different APIs

## IN CONTEXT
# Use preprocessor directives (with the macros defined in cmdline args) to conditionally include header files
# Make header file that includes support for each API (No CUDA, MKL, etc. code in context!!)
# context should provide an api that selects the device to run on




# // https://stackoverflow.com/questions/142877/can-the-c-preprocessor-be-used-to-tell-if-a-file-exists
# #if __has_include("cuda.h")
# #include <cuda.h>
# #define CUDA_EXISTS 1
# #endif


OS_DET=UNKNOWN
CPU_DET=UNKNOWN

ifeq ($(OS),Windows_NT)
	OS_DET=WIN
    CCFLAGS += -D WIN32
    ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
        CCFLAGS += -D AMD64
    else
        ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
            CCFLAGS += -D AMD64
        endif
        ifeq ($(PROCESSOR_ARCHITECTURE),x86)
            CCFLAGS += -D IA32
        endif
    endif
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
		OS_DET=LINUX
        CCFLAGS += -D LINUX
    endif
    ifeq ($(UNAME_S),Darwin)
		OS_DET=MACOSX
        CCFLAGS += -D OSX
    endif
    UNAME_P := $(shell uname -p)
    ifeq ($(UNAME_P),x86_64)
		CPU_DET=AMD64
        CCFLAGS += -D AMD64
    endif
    ifneq ($(filter %86,$(UNAME_P)),)
		CPU_DET=IA32
        CCFLAGS += -D IA32
    endif
    ifneq ($(filter arm%,$(UNAME_P)),)
		CPU_DET=ARM
        CCFLAGS += -D ARM
    endif
endif

check_os_arch:
	echo OS:  $(OS_DET) ARCH: $(CPU_DET)

mesh: main.c
	gcc -ldl main.c
	./a.out





mesh.txt: mesh
	./a.out > mesh.txt

hello: hello.cu
	nvcc hello.cu -o hello

clean:
	rm *.txt