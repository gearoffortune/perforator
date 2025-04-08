GO_LIBRARY()

LICENSE(BSD-3-Clause)

VERSION(v0.30.1-0.20250221230316-5055f70f240c)

SRCS(
    doc.go
    external.go
    golist.go
    golist_overlay.go
    loadmode_string.go
    packages.go
    visit.go
)

END()

RECURSE(
    gopackages
    internal
    packagestest
)
