# Builds mesh based on OS, ARCH, and known devices

# Set compiler, and base flags
CC := gcc
CCFLAGS := -ldl -O2

# Check for CUDA, and set CUDA_enabled flag
# This could be extended to support cuda runtime
NVCC_RESULT := $(shell which nvcc 2> NULL)
NVCC_TEST := $(notdir $(NVCC_RESULT))
ifeq ($(NVCC_TEST),nvcc)
    CUDA_enabled := 1
    CCFLAGS+= -D CUDA_enabled=1
else
    CUDA_enabled := 0
    CCFLAGS+= -D CUDA_enabled=0
endif

# Determine OS and architecture
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

# Get OS, and CPU architecture of host
check_host:
	@echo "OS: $(OS_DET)"
	@echo "ARCH: $(CPU_DET)"
	@echo "CUDA_enabled: $(CUDA_enabled)"
	@rm NULL 
    # Not sure what's producing this NULL file

# Build and run test
test:
	$(CC) -Wall src/test.c src/Mesh.c src/Device.c src/utils/String_H.c src/ops/CUDA_ops.c src/ops/NATURAL_ops.c -o a.out $(CCFLAGS)
	@./a.out > out.txt
	@cat out.txt

apitest:
	gcc -c -fPIC src/APIs/ctest.c -o src/APIs/testfile1.o
	gcc -c -fPIC src/APIs/ctesthello.c -o src/APIs/testfile2.o

	gcc -shared src/APIs/testfile1.o src/APIs/testfile2.o -o src/APIs/shared_lib/libmylib.so

find_headers:
	find src -name "*.h"

api:
	@mkdir -p objectfiles
	gcc -c -fPIC src/ops/CUDA_ops.h -o objectfiles/CUDA_ops.o
	gcc -c -fPIC src/ops/NATURAL_ops.h -o objectfiles/NATURAL_ops.o
	gcc -c -fPIC src/Device.h -o objectfiles/Device.o
	gcc -c -fPIC src/Mesh.h -o objectfiles/Mesh.o
	gcc -c -fPIC src/utils/String_H.h -o objectfiles/String_H.o

	gcc -shared objectfiles/Mesh.o objectfiles/String_H.o objectfiles/CUDA_ops.o objectfiles/NATURAL_ops.o objectfiles/Device.o -o src/APIs/shared_lib/meshlib.so

# $ gcc -shared -Wl,-soname,testlib -o testlib.so -fPIC testlib.c

# # or... for Mac OS X 
# $ gcc -shared -Wl,-install_name,testlib.so -o testlib.so -fPIC testlib.c
# https://stackoverflow.com/questions/5081875/ctypes-beginner

clean_api:
	rm -f src/APIs/shared_lib/*.so src/APIs/*.o

# Clean up
clean:
	@rm -f out.txt a.out NULL *.o *.so 
	@rm -rf objectfiles
	@echo "Cleaning complete"
	@echo "    This only cleans at a depth of 1, please see other clean methods if needed"

