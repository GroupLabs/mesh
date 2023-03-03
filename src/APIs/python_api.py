import ctypes

# Load the math library
math_lib = ctypes.CDLL('libm.so.6')
mesh_lib = ctypes.CDLL('shared_lib/libmylib.so')


# Define the function signatures
math_lib.cos.argtypes = [ctypes.c_double]
math_lib.cos.restype = ctypes.c_double

math_lib.sin.argtypes = [ctypes.c_double]
math_lib.sin.restype = ctypes.c_double

# Call the C functions from Python
print(math_lib.cos(0.5))
print(math_lib.sin(0.5))

mesh_lib.print_hello()

print(mesh_lib.add(1,2))