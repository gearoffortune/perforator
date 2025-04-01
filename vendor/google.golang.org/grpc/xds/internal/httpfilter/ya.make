GO_LIBRARY()

LICENSE(Apache-2.0)

VERSION(v1.69.4)

SRCS(
    httpfilter.go
)

END()

RECURSE(
    fault
    rbac
    router
)
