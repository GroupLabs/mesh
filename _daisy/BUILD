load("@rules_proto//proto:defs.bzl", "proto_library")
load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")

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
    deps = [":messaging_cc_proto"],
)

genrule(
    name = "model_and_tensor_flatbuffers",
    srcs = ["flatbuffers/model_and_tensor.fbs"],
    outs = ["model_and_tensor_generated.h"],
    cmd = "$(location @flatc_prebuilt//:flatc) --cpp --gen-object-api --no-includes -o $(RULEDIR) $(location flatbuffers/model_and_tensor.fbs)",
    tools = ["@flatc_prebuilt//:flatc"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "model_and_tensor_lib",
    srcs = ["model_and_tensor_generated.h"],
    hdrs = ["model_and_tensor_generated.h"],
    deps = ["@flatbuffers//:flatbuffers_lib"],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "networking",
    srcs = ["src/networking.cc"],
    deps = [
        "@com_github_grpc_grpc//:grpc++",
        ":messaging_cc_grpc",
        ":model_and_tensor_lib",
    ],
)

cc_binary(
    name = "inference",
    srcs = ["src/inference.cc"],
    deps = [
        "@libtorch//:libtorch",
    ],
    copts = ["-std=c++17"],
    linkopts = [
    ],
)
