#include <iostream>
#include <grpcpp/grpcpp.h>
#include "model_and_tensor.grpc.fb.h"
#include "model_and_tensor_generated.h"

int main(int argc, char** argv) {
    // Create a channel connected to the server.
    auto channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
    auto stub = myservice::ModelService::NewStub(channel);

    grpc::ClientContext context;
    flatbuffers::grpc::Message<myservice::ReceiveModelAndTensorResponse> response_msg;

    // Begin the client-streaming RPC.
    std::unique_ptr<grpc::ClientWriter<flatbuffers::grpc::Message<myservice::InputData>>> writer(
        stub->ReceiveModelAndTensor(&context, &response_msg));

    // 1. Send an InputData message with a FileChunk
    {
        flatbuffers::grpc::MessageBuilder builder;

        // Create some dummy file data
        uint8_t file_bytes[4] = {1, 2, 3, 4};
        auto file_data = builder.CreateVector(file_bytes, 4);
        auto file_chunk = myservice::CreateFileChunk(builder, file_data);

        // Note: The InputData schema is something like:
        // table InputData {
        //   file: FileChunk;
        //   tensor: TensorChunk;
        // }

        auto input_data = myservice::CreateInputData(builder, file_chunk, 0);
        builder.Finish(input_data);

        auto message = builder.ReleaseMessage<myservice::InputData>();
        if (!writer->Write(message)) {
            std::cerr << "Failed to write the first InputData message.\n";
        } else {
            std::cout << "Sent first message with FileChunk.\n";
        }
    }

    // 2. Send another InputData message with a TensorChunk
    {
        flatbuffers::grpc::MessageBuilder builder;

        // Create some dummy tensor data
        float tensor_values[3] = {1.0f, 2.0f, 3.0f};
        auto tensor_data = builder.CreateVector(tensor_values, 3);
        auto tensor_chunk = myservice::CreateTensorChunk(builder, tensor_data);

        auto input_data = myservice::CreateInputData(builder, 0, tensor_chunk);
        builder.Finish(input_data);

        auto message = builder.ReleaseMessage<myservice::InputData>();
        if (!writer->Write(message)) {
            std::cerr << "Failed to write the second InputData message.\n";
        } else {
            std::cout << "Sent second message with TensorChunk.\n";
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
