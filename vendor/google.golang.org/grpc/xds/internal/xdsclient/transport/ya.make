GO_LIBRARY()

LICENSE(Apache-2.0)

VERSION(v1.69.4)

SRCS(
    transport_interface.go
)

END()

RECURSE(
    ads
    grpctransport
    lrs
)
