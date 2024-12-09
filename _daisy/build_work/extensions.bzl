load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# Libtorch dependency implementation
def _libtorch_dependency_impl(ctx):
    http_archive(
        name = "libtorch",
        urls = ["https://download.pytorch.org/libtorch/cpu/libtorch-macos-arm64-2.4.0.zip"],
        strip_prefix = "libtorch",
        build_file_content = """
cc_library(
    name = "libtorch",
    srcs = glob([
        "lib/libc10.dylib",
        "lib/libfbjni.dylib",
        "lib/libomp.dylib",
        "lib/libpytorch_jni.dylib",
        "lib/libshm.dylib",
        "lib/libtorch.dylib",
        "lib/libtorch_cpu.dylib",
        "lib/libtorch_global_deps.dylib",
        "lib/*.so",
        "lib/*.dll",
        "lib/*.a",
    ], allow_empty = True),
    hdrs = glob([
        "include/**/*.h",
        "include/**/*.hpp",
        "include/**/*.cuh",
    ], allow_empty = True),
    includes = [
        "include",
        "include/torch/csrc/api/include",
    ],
    strip_include_prefix = "include",
    visibility = ["//visibility:public"],
    copts = [
        "-std=c++17",
    ],
)

toolchain(
    name = "toolchain",
    toolchain = ":libtorch",
    toolchain_type = "@bazel_tools//tools/cpp:toolchain_type",
    visibility = ["//visibility:public"],
)
"""
    )

libtorch_dependency = module_extension(
    implementation = _libtorch_dependency_impl,
)

# Flatc dependency implementation
def _flatc_dependency_impl(ctx):
    http_archive(
        name = "flatc_prebuilt",
        urls = ["https://github.com/google/flatbuffers/releases/download/v24.3.25/Mac.flatc.binary.zip"],
        strip_prefix = "",
        build_file_content = """
exports_files(["flatc"])
"""
    )

flatc_dependency = module_extension(
    implementation = _flatc_dependency_impl,
)

def _protoc_gen_validate_impl(ctx):
    http_archive(
        name = "protoc_gen_validate",
        urls = ["https://github.com/envoyproxy/protoc-gen-validate/archive/v1.0.4.tar.gz"],
        strip_prefix = "protoc-gen-validate-1.0.4",
        build_file_content = """
load("@rules_proto//proto:defs.bzl", "proto_library", "cc_proto_library")

proto_library(
    name = "validate_proto",
    srcs = glob(["validate/**/*.proto"]),
    visibility = ["//visibility:public"],
)
"""
    )

protoc_gen_validate_extension = module_extension(
    implementation = _protoc_gen_validate_impl,
)
