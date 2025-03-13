GO_LIBRARY()

LICENSE(BSD-3-Clause)

VERSION(v0.26.0)

SRCS(
    bimport.go
    exportdata.go
    gcimporter.go
    iexport.go
    iimport.go
    predeclared.go
    ureader_yes.go
)

END()

RECURSE(
    # gotest # st/YMAKE-102
)
