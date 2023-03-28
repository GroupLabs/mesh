# Builds mesh based on OS, ARCH, and known devices

# Set compiler, and base flags
CC := gcc
CCFLAGS := -ldl -O2
build_files := src/Mesh.c src/Device.c src/utils/string_H.c src/ops/NATURAL_ops.c

ERRORS := NONE
WARNS := NONE

# Determine OS and architecture
OS_DET=UNKNOWN_OS
PROC_DET=UNKNOWN_PROC

ifeq ($(OS),Windows_NT)
	  OS_DET=Windows
    ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
        PROC_DET=AMD64
    else
        ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
            PROC_DET=AMD64
        endif
        ifeq ($(PROCESSOR_ARCHITECTURE),x86)
            PROC_DET=86
        endif
    endif
else
    # OS implementation
    OS_DET := $(shell uname -s)

    # PROC architecture
    PROC_DET := $(shell uname -m)
endif

# Check for network devices
# TODO: Add support for network devices
# OS_DET += ...
# PROC_DET += ...

ifeq ($(OS_DET),Darwin)

    build_files += src/utils/darwin.c

    ### FRAMEWORKS

    # Check if Accelerate is enabled
    ifeq ($(PROC_DET),arm64)
        CCFLAGS += -D ACCELERATE_enabled=1
        # LDFLAGS += -framework Accelerate
        build_files += src/ops/ACCELERATE_ops.c
    else
        ifeq ($(shell sysctl -n machdep.cpu.brand_string | grep -i "apple" | wc -l), 1)
            WARNS += ROSETTA_MODE_WARN # Rosetta mode is enabled for terminal
        endif
    endif

    ### INTRINSICS

    # Check if NEON is enabled
    # sysctl -a | grep neon 
    # -mfpu=neon is not supported by clang (only gcc, armclang)

endif

ifeq ($(OS_DET),Linux)

    build_files += src/utils/linux.c # Specific files for each distro? (Is there a need for this?)

    #$(info "Being built")

    ### FRAMEWORKS

    # Check for CUDA, and set CUDA_enabled flag
    # This does not support, but can be extended to support CUDA runtime (cudart)
    # NVCC_RESULT := $(shell which nvcc 2> NULL)
    # NVCC_TEST := $(notdir $(NVCC_RESULT))
    # ifeq ($(NVCC_TEST),nvcc)
    #     CUDA_enabled := 1    
    # endif
    ifeq ($(shell which nvcc | grep -c nvcc),1)
        CCFLAGS += -D CUDA_enabled=1
        build_files += src/ops/CUDA_ops.c
    endif

    # Check for CUDA, and set CUDA_enabled flag
    ifeq ($(shell which dpcpp | grep -c dpcpp),1)
	CCFLAGS += -D ONEAPI_enabled=1
	build_files += src/ops/ONEAPI_ops.c
	#$(info "Confirmed oneapi Exists")
    endif

    ### INTRINSICS

    # Check if NEON is enabled
    ifeq ($(shell grep -c neon /proc/cpuinfo), 1)
        CCFLAGS += -D NEON_enabled=1
        build_files += src/ops/NEON_ops.c
    endif
endif

ifeq ($(OS_DET),Windows)

    build_files += src/utils/Windows.c # ?

    ### FRAMEWORKS

    # CUDA
    # OneAPI

    ### INTRINSICS

endif

# Handle unknown OS, and architecture
ifeq ($(OS_DET),UNKNOWN_OS)
    $(CCFLAGS) += -D UNKNOWN_OS=1
    WARNS += UNKNOWN_OS_WARN
endif

ifeq ($(PROC_DET),UNKNOWN_PROC)
    $(CCFLAGS) += -D UNKNOWN_PROC=1
    WARNS += UNKNOWN_PROC_WARN
endif

# Get OS, and CPU architecture of host
check_host:
	@echo "OS: $(OS_DET)"
	@echo "ARCH: $(PROC_DET)"
	@echo "ERRORS: $(ERRORS)"
	@echo "WARNS: $(WARNS)"
	@echo "CCFLAGS: $(CCFLAGS)"
	@echo "BUILD_FILES: $(build_files)"

# Build and run test
test:
	$(CC) src/tests/test.c $(build_files) -o temp.out $(CCFLAGS)
	@./temp.out > out.txt
	@cat out.txt

bindingtest:
	gcc -c -fPIC src/bindings/ctest.c -o src/bindings/testfile1.o
	gcc -c -fPIC src/bindings/ctesthello.c -o src/bindings/testfile2.o

	gcc -shared src/bindings/testfile1.o src/bindings/testfile2.o -o src/bindings/libmylib.so

binding:
	$(CC) -shared $(build_files) -o src/bindings/meshlib.so

clean_binding:
	rm -f src/bindings/*.so src/bindings/*.o

# Clean up
clean:
	@rm -f out.txt temp.out NULL *.o *.so 
	@rm -rf objectfiles
	@echo "Cleaning complete"
	@echo "    This only cleans at a depth of 1, please see other clean methods if needed"

