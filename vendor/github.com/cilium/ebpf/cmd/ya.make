IF (OS_LINUX)
    RECURSE(
        bpf2go
    )
ENDIF()

IF (OS_DARWIN)
    RECURSE(
        bpf2go
    )
ENDIF()
