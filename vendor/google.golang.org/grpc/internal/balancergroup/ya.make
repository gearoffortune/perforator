GO_LIBRARY()

LICENSE(Apache-2.0)

VERSION(v1.69.4)

SRCS(
    balancergroup.go
    balancerstateaggregator.go
)

GO_TEST_SRCS(balancergroup_test.go)

END()

RECURSE(
    gotest
)
