LIBRARY()

ADDINCL(
    ${ARCADIA_BUILD_ROOT}/contrib/libs/llvm18/lib/Target/X86
)

PEERDIR(
    contrib/libs/llvm18/include
    contrib/libs/llvm18/lib/Object
    contrib/libs/re2

    perforator/lib/elf
    perforator/lib/tls/parser
    perforator/lib/llvmex
    perforator/lib/python/asm/x86-64
)

SRCS(
    python.cpp
)

END()

RECURSE(
    asm
    cli
)

RECURSE_FOR_TESTS(
    ut
)
