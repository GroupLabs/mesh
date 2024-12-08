_daisy is the name of the project built for distributed inference. This will later be integrated into Mesh, to serve as the compute infrastructure. We’re building in cpp for portability.

> Note: This is a work in progress!

0. Download a flatc binary from `https://github.com/google/flatbuffers/releases/` and place it in the `_daisy` directory.

1. Install Bazel

2. Build with:

`bazel build --enable_bzlmod=false //:networking`

3. Run with:

`bazel-bin/networking`

`bazel run --enable_bzlmod=false //:networking`

4. Test with:

`bazel run --enable_bzlmod=false //:tests`

5. Run an inference with:

`bazel build --enable_bzlmod=false //:inference`
`bazel-bin/inference <path/to/model>/<model_name>.pt`

6. Clean with:

`bazel clean --expunge `


## Other

Generate the gRPC code:
protoc -I=proto --cpp_out=src proto/messaging.proto
protoc -I=proto --grpc_out=src --plugin=protoc-gen-grpc=$(which grpc_cpp_plugin) proto/messaging.proto -->
