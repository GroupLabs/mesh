tinygrad

1. Define appropriate operations with NEON/Accelerate 

    Need to implement the following operations:

    Use conda env to force python 3.8
    
    ***** Build PyTorch from source
    1. Build from source
        a. python3 setup.py develop
        
        There's an error on MacOS that involves not having Python dev libs not being
        installed, simply run `brew install python3`

    2. Create build file for chosen build system. PyTorch reccomends cmake.
    3. Run

    Example code:
    https://github.com/pytorch/examples/tree/main/cpp


    Writing op extensions for PyTorch:
    https://pytorch.org/tutorials/advanced/cpp_extension.html


    A. Buffer                                                       # class of memory on this device

        The buffer is the tensor struct.

        Adding aten from pytorch? Use test cases
        https://github.com/pytorch/pytorch/tree/master/aten/src/ATen/core
        https://github.com/pytorch/pytorch/blob/master/aten/src/ATen/core/Tensor.cpp
        https://github.com/pytorch/pytorch/blob/master/aten/src/ATen/core/Tensor.h

        Registering Mesh operations on ATen (maybe we can one shot the whole process):
        https://github.com/pytorch/pytorch/tree/master/aten/src/ATen/core/op_registration
        https://github.com/pytorch/pytorch/blob/master/aten/src/ATen/native/README.md

        https://pytorch-dev-podcast.simplecast.com/episodes/torch-vs-aten-apis
        https://pytorch.org/#community-module
        https://openai.com/research/triton
        https://dev-discuss.pytorch.org/t/tracing-with-primitives-update-0/577
        https://medium.com/@MaziBoustani/pytorch-2-0-release-explained-b5f167b86819

        References:
        https://github.com/ggerganov/ggml/blob/master/include/ggml/ggml.h

    B. unary_op  (NOOP, EXP, LOG, CAST)                             # A -> A
    C. reduce_op (SUM, MAX)                                         # A -> B (smaller size, B has 1 in shape)
    D. binary_op (ADD, SUB, MUL, DIV, POW, CMPEQ, MAX)              # A + A -> A (all the same size)
    E. movement_op (EXPAND, RESHAPE, PERMUTE, PAD, SHRINK, STRIDE)  # A -> B (different size)

2. Python bindings
3. Apply bindings to tinygrad

Bind in the same form as:
https://github.com/geohot/tinygrad/blob/master/tinygrad/runtime/ops_torch.py

4. Profile




micrograd

1. Reimplement