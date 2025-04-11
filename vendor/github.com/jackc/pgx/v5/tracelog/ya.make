GO_LIBRARY()

LICENSE(MIT)

VERSION(v5.7.4)

SRCS(
    tracelog.go
)

GO_XTEST_SRCS(
    # tracelog_test.go
)

END()

RECURSE(
    gotest
)
