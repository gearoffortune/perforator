GO_LIBRARY()

LICENSE(BSD-3-Clause)

VERSION(v0.26.0)

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
