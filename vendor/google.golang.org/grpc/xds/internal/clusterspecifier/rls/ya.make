GO_LIBRARY()

LICENSE(Apache-2.0)

VERSION(v1.69.4)

SRCS(
    rls.go
)

GO_TEST_SRCS(rls_test.go)

END()

RECURSE(
    gotest
)
