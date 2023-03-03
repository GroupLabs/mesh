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
test: src/test.c
	$(CC) $(CCFLAGS) src/test.c 
	./a.out > out.txt
	cat out.txt

api:
	cc -fPIC -shared -o Mesh.so src/Mesh.h

# Clean up
clean:
	rm out.txt a.out NULL
