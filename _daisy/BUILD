load("@rules_proto//proto:defs.bzl", "proto_library")
load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")

# Proto Definitions
proto_library(
    name = "messaging_proto",
    srcs = ["proto/messaging.proto"],
)

cc_proto_library(
    name = "messaging_cc_proto",
    deps = [":messaging_proto"],
)

cc_grpc_library(
    name = "messaging_cc_grpc",
    srcs = [":messaging_proto"],
    grpc_only = True,
    deps = [
        ":messaging_cc_proto",
        "@com_github_grpc_grpc//:grpc++",
        "@com_google_protobuf//:protobuf",  # Added Protobuf dependency
    ],
)

# FlatBuffers Genrule
genrule(
    name = "model_and_tensor_flatbuffers",
    srcs = ["flatbuffers/model_and_tensor.fbs"],
    outs = [
        "model_and_tensor_generated.h",
        "model_and_tensor.grpc.fb.h",
        "model_and_tensor.grpc.fb.cc",
    ],
    cmd = "$(location @flatc_prebuilt//:flatc) --cpp --grpc --gen-object-api -o $(RULEDIR) $(location flatbuffers/model_and_tensor.fbs)",
    tools = ["@flatc_prebuilt//:flatc"],
    visibility = ["//visibility:public"],
)

# FlatBuffers Generated Code Library
cc_library(
    name = "model_and_tensor_lib",
    srcs = ["model_and_tensor.grpc.fb.cc"],
    hdrs = [
        "model_and_tensor.grpc.fb.h",
        "model_and_tensor_generated.h",
    ],
    deps = [
        "@flatbuffers//:flatbuffers_lib",
        "@com_github_grpc_grpc//:grpc++",
        "@com_google_protobuf//:protobuf",  # Added Protobuf dependency
    ],
    copts = ["-std=c++17"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "log",
    srcs = ["src/log.cc"],
    hdrs = ["include/log.h"],
    visibility = ["//visibility:public"],
    copts = ["-std=c++17"],
)

# Networking Binary
cc_binary(
    name = "networking",
    srcs = ["src/networking.cc"],
    deps = [
        ":log",
        "@com_github_grpc_grpc//:grpc++",
        ":messaging_cc_grpc",
        ":model_and_tensor_lib",
        "@libtorch//:libtorch",
    ],
    copts = ["-std=c++17"],
)

# test flatbuffers test
cc_binary(
    name = "test-fbs",
    srcs = ["tests/flatbuffers-test-client.cc"],
    deps = [
        "@com_github_grpc_grpc//:grpc++",
        ":messaging_cc_grpc",
        ":model_and_tensor_lib",
    ],
    copts = ["-std=c++17"],
    data = ["torchscript/simple_model.pt"],
)

# Inference Binary
cc_binary(
    name = "inference",
    srcs = ["src/inference.cc"],
    deps = [
        "@libtorch//:libtorch",
    ],
    copts = ["-std=c++17"],
    linkopts = [
        # Add any necessary link options here
    ],
)
