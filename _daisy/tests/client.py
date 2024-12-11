import grpc
import flatbuffers
import os
import sys
import numpy as np

# Import FlatBuffers-generated classes
from myservice.FileChunk import FileChunk
from myservice.TensorChunk import TensorChunk
from myservice.InputData import InputData
from myservice.ReceiveModelAndTensorResponse import ReceiveModelAndTensorResponse


def serialize_file_chunk(file_path, chunk_size=1024):
    """
    Generator that reads the .pt file in chunks and yields serialized FileChunk FlatBuffers bytes.
    """
    if not os.path.isfile(file_path):
        raise FileNotFoundError(f"The .pt file at '{file_path}' was not found.")

    with open(file_path, 'rb') as f:
        while True:
            file_chunk = f.read(chunk_size)
            if not file_chunk:
                break

            builder = flatbuffers.Builder(0)

            # Serialize FileChunk
            FileChunk.FileChunkStart(builder)
            data_vector = FileChunk.CreateDataVector(builder, list(file_chunk))
            FileChunk.FileChunkAddData(builder, data_vector)
            file_chunk_offset = FileChunk.FileChunkEnd(builder)

            builder.Finish(file_chunk_offset)

            # Get the bytes
            buf = builder.Output()
            yield buf


def serialize_tensor(tensor_data, chunk_size=1024):
    """
    Generator that serializes tensor data in chunks and yields serialized TensorChunk FlatBuffers bytes.
    """
    tensor_flat = tensor_data.flatten().tolist()
    for i in range(0, len(tensor_flat), chunk_size):
        chunk = tensor_flat[i:i + chunk_size]

        builder = flatbuffers.Builder(0)

        # Serialize TensorChunk
        TensorChunk.TensorChunkStart(builder)
        data_vector = TensorChunk.CreateDataVector(builder, chunk)
        TensorChunk.TensorChunkAddData(builder, data_vector)
        tensor_chunk_offset = TensorChunk.TensorChunkEnd(builder)

        builder.Finish(tensor_chunk_offset)

        # Get the bytes
        buf = builder.Output()
        yield buf


def generate_input_data(pt_file_path, tensor_data, file_chunk_size=1024, tensor_chunk_size=1024):
    """
    Generator that yields serialized InputData FlatBuffers messages containing FileChunk or TensorChunk payloads.
    """
    # Serialize and send FileChunks
    for file_chunk_buf in serialize_file_chunk(pt_file_path, file_chunk_size):
        builder = flatbuffers.Builder(0)

        # Serialize InputData with FileChunk
        InputData.InputDataStart(builder)
        InputData.InputDataAddFile(builder, FileChunk.FileChunkEnd(builder))
        input_data_offset = InputData.InputDataEnd(builder)

        builder.Finish(input_data_offset)

        # Get the bytes
        buf = builder.Output()
        yield buf

    # Serialize and send TensorChunks
    for tensor_chunk_buf in serialize_tensor(tensor_data, tensor_chunk_size):
        builder = flatbuffers.Builder(0)

        # Serialize InputData with TensorChunk
        InputData.InputDataStart(builder)
        InputData.InputDataAddTensor(builder, TensorChunk.TensorChunkEnd(builder))
        input_data_offset = InputData.InputDataEnd(builder)

        builder.Finish(input_data_offset)

        # Get the bytes
        buf = builder.Output()
        yield buf


def run_flatbuffers_client(pt_file_path, tensor_data, server_address="0.0.0.0:50051"):
    """
    Sends FlatBuffers serialized FileChunk and TensorChunk data to the gRPC server.
    """
    # Create a gRPC channel
    channel = grpc.insecure_channel(server_address)

    # Define the RPC method name (fully qualified)
    method_name = "/myservice.ModelService/ReceiveModelAndTensor"

    # Create a stream_unary RPC call
    rpc = channel.stream_unary(method_name)

    # Create the request iterator
    request_iterator = generate_input_data(pt_file_path, tensor_data)

    try:
        # Send the stream and get the response
        response = rpc(request_iterator)

        # Deserialize the response FlatBuffers message
        response_message = ReceiveModelAndTensorResponse.ReceiveModelAndTensorResponse.GetRootAsReceiveModelAndTensorResponse(response, 0)
        print("Server Response Status:", response_message.Status())
    except grpc.RpcError as e:
        print(f"gRPC Error: {e.code()} - {e.details()}")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python client.py <path_to_pt_file> <tensor_size>")
        print("Example: python client.py model.pt 1000")
        sys.exit(1)

    pt_file_path = sys.argv[1]
    try:
        tensor_size = int(sys.argv[2])
    except ValueError:
        print("Error: Tensor size must be an integer.")
        sys.exit(1)

    # Verify the .pt file exists
    if not os.path.isfile(pt_file_path):
        print(f"Error: The file '{pt_file_path}' does not exist.")
        sys.exit(1)

    # Generate example tensor data
    tensor_data = np.random.rand(tensor_size).astype(np.float32)

    # Run the client
    run_flatbuffers_client(pt_file_path, tensor_data)
