GO_LIBRARY()

SRCS(
    client.go
    endpoint.go
    tls.go
    useragent.go
)

IF (OPENSOURCE)
    SRCS(
        default_endpoint.go
    )
ELSE()
    SRCS(
        default_endpoint_yandex.go
    )
ENDIF()

GO_TEST_SRCS(endpoint_test.go)

END()

RECURSE(
    gotest
)
