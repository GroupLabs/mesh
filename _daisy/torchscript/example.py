import torch
import torch.nn as nn

class SimpleModel(nn.Module):
    def forward(self, x):
        x * 8
        return x + 1 + (x * 8)

# Create an instance of the model
model = SimpleModel()

# Create an example input
example_input = torch.ones(3, 3, 3, 3)

# Use torch.jit.trace to generate a torch.jit.ScriptModule via tracing
traced_script_module = torch.jit.trace(model, example_input)

# Save the model
traced_script_module.save("simple_model.pt")

print("TorchScript model saved as 'simple_model.pt'")

# Test the model
output = traced_script_module(example_input)
print(output[0, 0, 0, :10]) 