GO_LIBRARY()

LICENSE(Apache-2.0)

VERSION(v1.31.0)

SRCS(
    gen.go
)

END()

RECURSE(
    env
    internaltest
    matchers
    x
)
