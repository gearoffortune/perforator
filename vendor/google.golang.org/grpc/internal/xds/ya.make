GO_LIBRARY()

LICENSE(Apache-2.0)

VERSION(v1.69.4)

SRCS(
    xds.go
)

END()

RECURSE(
    bootstrap
    matcher
    rbac
)
