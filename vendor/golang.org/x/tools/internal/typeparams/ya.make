GO_LIBRARY()

LICENSE(BSD-3-Clause)

VERSION(v0.30.1-0.20250221230316-5055f70f240c)

SRCS(
    common.go
    coretype.go
    free.go
    normalize.go
    termlist.go
    typeterm.go
)

END()

RECURSE(
    genericfeatures
)
