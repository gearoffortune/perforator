GO_LIBRARY()

LICENSE(Apache-2.0)

VERSION(v1.69.4)

SRCS(
    admin.go
)

GO_XTEST_SRCS(admin_test.go)

END()

RECURSE(
    gotest
    test
)
