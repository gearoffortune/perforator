GO_LIBRARY()

LICENSE(BSD-3-Clause)

VERSION(v0.30.1-0.20250221230316-5055f70f240c)

SRCS(
    invoke.go
    vendor.go
    version.go
)

IF (OS_LINUX)
    SRCS(
        invoke_unix.go
    )
ENDIF()

IF (OS_DARWIN)
    SRCS(
        invoke_unix.go
    )
ENDIF()

IF (OS_WINDOWS)
    SRCS(
        invoke_notunix.go
    )
ENDIF()

END()
