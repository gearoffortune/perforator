GO_LIBRARY()

LICENSE(Apache-2.0)

VERSION(v1.31.0)

SRCS(
    cloud-function.go
    cloud-run.go
    detector.go
    gce.go
    gke.go
    types.go
    version.go
)

GO_TEST_SRCS(
    cloud-function_test.go
    cloud-run_test.go
    detector_test.go
)

END()

RECURSE(
    gotest
)
