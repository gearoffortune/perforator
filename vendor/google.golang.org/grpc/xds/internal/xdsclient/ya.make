GO_LIBRARY()

LICENSE(Apache-2.0)

VERSION(v1.69.4)

SRCS(
    attributes.go
    authority.go
    channel.go
    client.go
    client_new.go
    client_refcounted.go
    clientimpl.go
    clientimpl_dump.go
    clientimpl_loadreport.go
    clientimpl_watchers.go
    logging.go
    requests_counter.go
)

GO_TEST_SRCS(
    channel_test.go
    client_refcounted_test.go
    # client_test.go
    # requests_counter_test.go
)

GO_XTEST_SRCS(
    # xdsclient_test.go
)

END()

RECURSE(
    # e2e_test
    # gotest
    internal
    load
    tests
    transport
    xdslbregistry
    xdsresource
)
