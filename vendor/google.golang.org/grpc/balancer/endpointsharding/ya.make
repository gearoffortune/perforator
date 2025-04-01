GO_LIBRARY()

LICENSE(Apache-2.0)

VERSION(v1.69.4)

SRCS(
    endpointsharding.go
)

GO_TEST_SRCS(endpointsharding_test.go)

END()

RECURSE(
    gotest
)
