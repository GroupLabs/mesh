from ctypes import CDLL

so_file = "Mesh.so"

Mesh = CDLL(so_file)

print(type(Mesh))
