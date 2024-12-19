load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def _grpc_impl(module_ctx):
    module_ctx.call(
        "@bazel_tools//tools/build_defs/repo:http.bzl%http_archive",
        name = "com_github_grpc_grpc",
        urls = ["https://github.com/grpc/grpc/archive/b8a04acbbf18fd1c805e5d53d62ed9fa4721a4d1.tar.gz"],
        strip_prefix = "grpc-b8a04acbbf18fd1c805e5d53d62ed9fa4721a4d1",
    )

grpc_extension = module_extension(
    implementation = _grpc_impl,
)

def _grpc_all_deps_impl(ctx):
    # This extension replicates what grpc_deps() and grpc_extra_deps() would have done.
    # For demonstration, we add protobuf and zlib as typical gRPC deps.

    http_archive(
        name = "protobuf",
        urls = ["https://github.com/protocolbuffers/protobuf/archive/v3.21.3.tar.gz"],
        strip_prefix = "protobuf-3.21.3",
        build_file_content = """
cc_library(
    name = "protobuf",
    srcs = glob(["src/*.cc"]),
    hdrs = glob(["src/*.h", "include/**/*.h"]),
    includes = ["include"],
    visibility = ["//visibility:public"],
)
"""
    )

    http_archive(
        name = "zlib",
        urls = ["https://github.com/madler/zlib/archive/refs/tags/v1.2.11.tar.gz"],
        strip_prefix = "zlib-1.2.11",
        build_file_content = """
cc_library(
    name = "zlib",
    srcs = glob(["*.c"]),
    hdrs = glob(["*.h"]),
    visibility = ["//visibility:public"],
)
"""
    )

    # Add any additional dependencies you need that grpc_deps() and grpc_extra_deps() would handle.
    # For example, cares or openssl could also be added here if needed.

grpc_all_deps_extension = module_extension(
    implementation = _grpc_all_deps_impl,
)

def _libtorch_impl(ctx):
    http_archive(
        name = "libtorch",
        urls = ["https://download.pytorch.org/libtorch/cpu/libtorch-macos-arm64-2.4.0.zip"],
        strip_prefix = "libtorch",
        build_file_content = """
cc_library(
    name = "libtorch",
    srcs = glob([
        "lib/*.dylib",
        "lib/*.so",
        "lib/*.dll",
        "lib/*.a",
    ], allow_empty=True),
    hdrs = glob([
        "include/**/*.h",
        "include/**/*.hpp",
        "include/**/*.cuh",
    ], allow_empty=True),
    includes = [
        "include",
        "include/torch/csrc/api/include",
    ],
    strip_include_prefix = "include",
    visibility = ["//visibility:public"],
    copts = ["-std=c++17"],
)
"""
    )

libtorch_extension = module_extension(
    implementation = _libtorch_impl,
)

def _flatbuffers_impl(ctx):
    http_archive(
        name = "flatc_prebuilt",
        urls = ["https://github.com/google/flatbuffers/releases/download/v24.3.25/Mac.flatc.binary.zip"],
        strip_prefix = "",
        build_file_content = """
exports_files(["flatc"])
"""
    )

flatbuffers_extension = module_extension(
    implementation = _flatbuffers_impl,
)
