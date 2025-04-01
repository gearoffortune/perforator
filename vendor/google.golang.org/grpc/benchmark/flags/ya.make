GO_LIBRARY()

LICENSE(Apache-2.0)

VERSION(v1.69.4)

SRCS(
    flags.go
)

GO_TEST_SRCS(flags_test.go)

END()

RECURSE(
    gotest
)
