The following psuedo code is meant to develop an intuition as to how Mesh will be used.

    Given two arrays:

    a = [1, 2, 3, 4, 5, 6, 8, 9, 10]

    b = [1, 2, 3, 4, 5, 6, 8, 9, 10]

    We can implement operations like:

    mesh = Mesh() // initiate Mesh context, automatically 

    print(mesh.devices()) // get available devices

    product = mesh.multiply(a, b) // array multiplication

    tensor = mesh.multiply(product, product) // matrix multiplication

    sum = mesh.add(tensor, tensor) // tensor addition