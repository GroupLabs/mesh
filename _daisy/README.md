_daisy is the name of the project built for distributed inference. This will later be integrated into Mesh, to serve as the compute infrastructure. We’re building in cpp for portability.

> Note: This is a work in progress!

1. Install Bazel

2. Build with:

`bazel build --enable_bzlmod=false //:main`

3. Run with:

`bazel-bin/main`

`bazel run --enable_bzlmod=false //:main`

4. Test with:

`bazel test //:tests`

5. Clean with:

`bazel clean --expunge `


## Other

Generate the gRPC code:
protoc -I=proto --cpp_out=src proto/messaging.proto
protoc -I=proto --grpc_out=src --plugin=protoc-gen-grpc=$(which grpc_cpp_plugin) proto/messaging.proto -->
