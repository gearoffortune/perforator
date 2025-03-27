LIBRARY()

PEERDIR(
    contrib/libs/llvm18/include
    contrib/libs/llvm18/lib/Object

    perforator/lib/llvmex
)

SRCS(
    elf.cpp
)

END()
