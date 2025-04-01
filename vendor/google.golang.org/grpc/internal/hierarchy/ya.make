GO_LIBRARY()

LICENSE(Apache-2.0)

VERSION(v1.69.4)

SRCS(
    hierarchy.go
)

GO_TEST_SRCS(hierarchy_test.go)

END()

RECURSE(
    gotest
)
