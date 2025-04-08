GO_LIBRARY()

LICENSE(BSD-3-Clause)

VERSION(v0.30.1-0.20250221230316-5055f70f240c)

SRCS(
    bimport.go
    exportdata.go
    gcimporter.go
    iexport.go
    iimport.go
    iimport_go122.go
    predeclared.go
    support.go
    ureader_yes.go
)

END()

RECURSE(
    # gotest # st/YMAKE-102
)
