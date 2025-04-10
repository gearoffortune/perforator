LIBRARY()

ADDINCL(
    ${ARCADIA_BUILD_ROOT}/contrib/libs/llvm18/lib/Target/X86
)

PEERDIR(
    contrib/libs/llvm18/include
    contrib/libs/llvm18/lib/Object

    perforator/lib/elf
    perforator/lib/llvmex
    perforator/lib/pthread/asm/x86
)

SRCS(
    pthread.cpp
)

END()

RECURSE(
    asm
    cli
)

IF (NOT OPENSOURCE)
    RECURSE_FOR_TESTS(ut)
ENDIF()
