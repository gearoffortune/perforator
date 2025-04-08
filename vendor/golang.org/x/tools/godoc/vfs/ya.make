GO_LIBRARY()

LICENSE(BSD-3-Clause)

VERSION(v0.30.1-0.20250221230316-5055f70f240c)

SRCS(
    emptyvfs.go
    fs.go
    namespace.go
    os.go
    vfs.go
)

END()

RECURSE(
    gatefs
    httpfs
    mapfs
    zipfs
)
