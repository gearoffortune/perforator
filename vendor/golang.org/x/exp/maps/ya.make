GO_LIBRARY()

LICENSE(BSD-3-Clause)

VERSION(v0.0.0-20240707233637-46b078467d37)

SRCS(
    maps.go
)

GO_TEST_SRCS(maps_test.go)

END()

RECURSE(
    gotest
)
