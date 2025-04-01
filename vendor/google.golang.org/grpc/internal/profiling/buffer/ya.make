GO_LIBRARY()

LICENSE(Apache-2.0)

VERSION(v1.69.4)

SRCS(
    buffer.go
)

GO_TEST_SRCS(buffer_test.go)

END()

RECURSE(
    gotest
)
