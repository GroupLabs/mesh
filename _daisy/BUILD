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

cc_binary(
    name = "main",
    srcs = ["src/main.cc"],
    deps = [
        "@com_github_grpc_grpc//:grpc++",
        ":messaging_cc_grpc",
    ],
)



cc_binary(
    name = "networking",
    srcs = ["src/networking.cc"],
    deps = [
        "@com_github_grpc_grpc//:grpc++",
        ":messaging_cc_grpc",
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
