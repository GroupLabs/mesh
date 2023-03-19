# Builds mesh based on OS, ARCH, and known devices

# Set compiler, and base flags
CC := gcc
CCFLAGS := -ldl -O2
build_files := src/Mesh.c src/Device.c src/utils/String_H.c src/ops/NATURAL_ops.c

# Error handling
ERRORS := NONE
WARNS := NONE

# Determine OS and architecture
OS_DET=UNKNOWN_OS
CPU_DET=UNKNOWN_CPU

ifeq ($(OS),Windows_NT)
	  OS_DET=WIN
    ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
        CPU_DET=AMD64
    else
        ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
            CPU_DET=AMD64
        endif
        ifeq ($(PROCESSOR_ARCHITECTURE),x86)
            CPU_DET=86
        endif
    endif
else
    # OS implementation
    OS_DET := $(shell uname -s)

    # CPU architecture
    CPU_DET := $(shell uname -m)
endif

# Check if Accelerate is enabled
ifndef ACCELERATE
    ACCELERATE_enabled=0
	ifeq ($(OS_DET),Darwin)
        ifeq ($(CPU_DET),arm64)
            ACCELERATE_enabled=1
        else
            ifeq ($(shell sysctl -n machdep.cpu.brand_string | grep -i "apple" | wc -l), 1)
                WARNS += ROSETTA_MODE_WARN # Rosetta mode is enabled for terminal
            endif
        endif
	endif
endif

# Check for CUDA, and set CUDA_enabled flag
# This does not support, but can be extended to support CUDA runtime (cudart)
NVCC_RESULT := $(shell which nvcc 2> NULL)
NVCC_TEST := $(notdir $(NVCC_RESULT))
ifeq ($(NVCC_TEST),nvcc)
    CUDA_enabled := 1
else
    CUDA_enabled := 0
endif

# Set enabled libraries
ifeq ($(ACCELERATE_enabled),1)
    CCFLAGS += -D ACCELERATE_enabled=1
    build_files += src/ops/ACCELERATE_ops.c src/utils/darwin.c
endif
ifeq ($(CUDA_enabled),1)
    CCFLAGS += -D CUDA_enabled=1
    build_files += src/ops/CUDA_ops.c
endif

# LDFLAGS += -framework Accelerate

# Get OS, and CPU architecture of host
check_host:
	@echo "OS: $(OS_DET)"
	@echo "ARCH: $(CPU_DET)"
	@echo "ERRORS: $(ERRORS)"
	@echo "WARNS: $(WARNS)"
	@echo "CCFLAGS: $(CCFLAGS)"
	@echo "BUILD_FILES: $(build_files)"
	@rm NULL 
    # Not sure what's producing this NULL file

# Build and run test
test:
	$(CC) src/tests/test.c $(build_files) -o temp.out $(CCFLAGS)
	@./temp.out > out.txt
	@cat out.txt

apitest:
	gcc -c -fPIC src/bindings/ctest.c -o src/bindings/testfile1.o
	gcc -c -fPIC src/bindings/ctesthello.c -o src/bindings/testfile2.o

	gcc -shared src/bindings/testfile1.o src/bindings/testfile2.o -o src/bindings/libmylib.so

api:
	$(CC) -shared $(build_files) -o src/bindings/meshlib.so

clean_api:
	rm -f src/bindings/*.so src/bindings/*.o

# Clean up
clean:
	@rm -f out.txt temp.out NULL *.o *.so 
	@rm -rf objectfiles
	@echo "Cleaning complete"
	@echo "    This only cleans at a depth of 1, please see other clean methods if needed"

