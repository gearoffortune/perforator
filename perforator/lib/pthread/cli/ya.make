PROGRAM()

ADDINCL(
    ${ARCADIA_BUILD_ROOT}/contrib/libs/llvm18/lib/Target/X86
)

PEERDIR(
    contrib/libs/llvm18/include
    contrib/libs/llvm18/lib/Target/X86
    contrib/libs/llvm18/lib/Object

    library/cpp/logger/global

    perforator/lib/pthread
)

SRCS(
    main.cpp
)

END()
