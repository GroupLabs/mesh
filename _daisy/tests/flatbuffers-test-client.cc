// This is an example of how to use flatbuffers in streaming mode to send a file, and a tensor.

#include <iostream>
#include <fstream>
#include <vector>
#include <grpcpp/grpcpp.h>
#include "model_and_tensor.grpc.fb.h"
#include "model_and_tensor_generated.h"

// Function to read file into a byte vector
std::vector<uint8_t> ReadFile(const std::string& file_path) {
    std::ifstream file(file_path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + file_path);
    }

    // Read file into buffer
    std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(file)),
                                 std::istreambuf_iterator<char>());
    return buffer;
}

int main(int argc, char** argv) {
    // Check if the file path is provided as a command-line argument
    std::string file_path = "torchscript/simple_model.pt"; // Default path
    if (argc > 1) {
        file_path = argv[1];
    }

    // Read the file
    std::vector<uint8_t> file_bytes;
    try {
        file_bytes = ReadFile(file_path);
        std::cout << "Successfully read file: " << file_path << " (" << file_bytes.size() << " bytes)\n";
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }

    // Create a channel connected to the server.
    auto channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
    auto stub = myservice::ModelService::NewStub(channel);

    grpc::ClientContext context;
    flatbuffers::grpc::Message<myservice::ReceiveModelAndTensorResponse> response_msg;

    // Begin the client-streaming RPC.
    std::unique_ptr<grpc::ClientWriter<flatbuffers::grpc::Message<myservice::InputData>>> writer(
        stub->ReceiveModelAndTensor(&context, &response_msg));

    {
        flatbuffers::grpc::MessageBuilder builder;

        std::cout << "Creating FileChunk. File size: " << file_bytes.size() << " bytes." << std::endl;

        // Optional: Log the first few bytes of the file for verification (e.g., first 10 bytes)
        std::cout << "File content (first 10 bytes): ";
        for (size_t i = 0; i < std::min<size_t>(10, file_bytes.size()); ++i) {
            std::cout << std::hex << static_cast<int>(file_bytes[i]) << " ";
        }
        std::cout << std::dec << std::endl; // Reset back to decimal formatting

        // Use the file_bytes read from the file
        auto file_data = builder.CreateVector(file_bytes.data(), file_bytes.size());
        auto file_chunk = myservice::CreateFileChunk(builder, file_data);

        std::cout << "Contents of FileChunk (first 10 bytes): ";
        for (size_t i = 0; i < std::min<size_t>(10, file_bytes.size()); ++i) {
            std::cout << std::hex << static_cast<int>(file_bytes[i]) << " ";
        }
        std::cout << std::dec << std::endl; // Reset to decimal formatting

        // Assuming tensor is not needed in this message; set to 0 or handle accordingly
        auto input_data = myservice::CreateInputData(builder, file_chunk, 0);
        builder.Finish(input_data);

        auto message = builder.ReleaseMessage<myservice::InputData>();
        if (!writer->Write(message)) {
            std::cerr << "Failed to write the InputData message with FileChunk.\n";
            return 1;
        } else {
            std::cout << "Sent message with FileChunk (" << file_bytes.size() << " bytes).\n";
        }
    }

    {
        flatbuffers::grpc::MessageBuilder builder;

        // Define tensor dimensions
        const int dim1 = 3;
        const int dim2 = 3;
        const int dim3 = 3;
        const int dim4 = 3;
        const int total_elements = dim1 * dim2 * dim3 * dim4;

        // Initialize tensor data (e.g., all ones)
        std::vector<float> tensor_values(total_elements, 1.0f);

        // Create a FlatBuffers vector from the tensor data
        auto tensor_data = builder.CreateVector(tensor_values);

        // Create a TensorChunk with the tensor data
        auto tensor_chunk = myservice::CreateTensorChunk(builder, tensor_data);

        // Assuming tensor is not needed in this message; set to 0 or handle accordingly
        auto input_data = myservice::CreateInputData(builder, 0, tensor_chunk);
        builder.Finish(input_data);

        auto message = builder.ReleaseMessage<myservice::InputData>();
        if (!writer->Write(message)) {
            std::cerr << "Failed to write the InputData message with FileChunk.\n";
            return 1;
        } else {
            std::cout << "Sent message with TensorChunk (" << tensor_values.size() << " bytes).\n";
        }
    }

    // Indicate that we are done sending messages.
    writer->WritesDone();

    // Complete the RPC and wait for the response.
    grpc::Status status = writer->Finish();
    if (!status.ok()) {
        std::cerr << "RPC failed: " << status.error_message() << "\n";
        return 1;
    }

    // Access the final response from the server
    const myservice::ReceiveModelAndTensorResponse* resp = response_msg.GetRoot();
    if (resp && resp->status()) {
        std::cout << "Server responded with status: " << resp->status()->str() << "\n";
    } else {
        std::cout << "No valid response received from the server.\n";
    }

    return 0;
}
