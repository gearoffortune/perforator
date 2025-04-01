GO_LIBRARY()

LICENSE(Apache-2.0)

VERSION(v1.69.4)

SRCS(
    google.go
    xds.go
)

GO_TEST_SRCS(
    google_test.go
    xds_test.go
)

END()

RECURSE(
    gotest
)
