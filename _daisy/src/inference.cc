#include <torch/script.h>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <path to TorchScript module>" << std::endl;
        return 1;
    }

    try {
        // Load the TorchScript module
        torch::jit::script::Module module = torch::jit::load(argv[1]);
        std::cout << "TorchScript module loaded successfully" << std::endl;

        // Create an example input tensor
        std::vector<torch::jit::IValue> inputs;
        inputs.push_back(torch::ones({3, 3, 3, 3}));

        // Run the model
        at::Tensor output = module.forward(inputs).toTensor();

        // Print the output
        std::cout << "Model output shape: " << output.sizes() << std::endl;
        std::cout << "First few values: " << output.slice(/*dim=*/1, /*start=*/0, /*end=*/5) << std::endl;

    }
    catch (const c10::Error& e) {
        std::cerr << "Error loading or running the model: " << e.msg() << std::endl;
        return 1;
    }

    return 0;
}