#include <torch/torch.h>
#include <iostream>

int main() {
  std::cout << "TESTING TORCH" << std::endl;
  std::cout << "Torch version: " << TORCH_VERSION << std::endl;
  torch::Tensor tensorA = torch::eye(3);
  torch::Tensor tensorB = torch::randint(0, 10, {3, 3});
  std::cout << "Tensor A: " << std::endl << tensorA << std::endl;
  std::cout << "Tensor B: " << std::endl << tensorB << std::endl;
  std::cout << "Tensor A + B: " << std::endl << tensorA + tensorB << std::endl;

  return 0;
}