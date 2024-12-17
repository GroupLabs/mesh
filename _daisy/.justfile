run:
    bazel run --enable_bzlmod=false //:networking

clean:
    bazel clean --expunge

test:
    bazel run --enable_bzlmod=false //:test-fbs
