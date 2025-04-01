GO_LIBRARY()

LICENSE(Apache-2.0)

VERSION(v1.69.4)

SRCS(
    client_metrics.go
    grpc_trace_bin_propagator.go
    opentelemetry.go
    server_metrics.go
)

GO_TEST_SRCS(
    grpc_trace_bin_propagator_test.go
    metricsregistry_test.go
)

GO_XTEST_SRCS(
    e2e_test.go
    example_test.go
)

END()

RECURSE(
    csm
    gotest
    internal
)
