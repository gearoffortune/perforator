GO_LIBRARY()

SRCS(
    dss.go
)

GO_XTEST_SRCS(dss_test.go)

END()

RECURSE(
    gotest
)
