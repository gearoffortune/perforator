GO_LIBRARY()

SRCS(
    listener.go
    operators.go
    parse_error.go
    parser.go
    utils.go
)

GO_XTEST_SRCS(
    parse_error_test.go
    parser_test.go
    utils_test.go
)

END()

RECURSE(
    generated
    gotest
)
