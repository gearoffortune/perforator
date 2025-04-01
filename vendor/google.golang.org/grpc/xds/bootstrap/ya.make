GO_LIBRARY()

LICENSE(Apache-2.0)

VERSION(v1.69.4)

SRCS(
    bootstrap.go
    credentials.go
)

GO_TEST_SRCS(bootstrap_test.go)

END()

RECURSE(
    gotest
)
