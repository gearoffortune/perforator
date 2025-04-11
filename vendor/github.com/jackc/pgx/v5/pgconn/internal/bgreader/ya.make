GO_LIBRARY()

LICENSE(MIT)

VERSION(v5.7.4)

SRCS(
    bgreader.go
)

GO_XTEST_SRCS(bgreader_test.go)

END()

RECURSE(
    gotest
)
