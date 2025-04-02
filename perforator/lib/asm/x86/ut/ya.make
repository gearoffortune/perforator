GTEST()

ADDINCL(
    ${ARCADIA_BUILD_ROOT}/contrib/libs/llvm18/lib/Target/X86
)

PEERDIR(
    contrib/libs/llvm18/include
    contrib/libs/llvm18/lib/Object
    contrib/libs/llvm18/lib/Support
    contrib/libs/llvm18/lib/Target
    contrib/libs/llvm18/lib/Target/X86
    contrib/libs/llvm18/lib/Target/X86/Disassembler
    contrib/libs/llvm18/lib/Target/X86/MCTargetDesc

    library/cpp/logger/global
    library/cpp/testing/gtest
    library/cpp/testing/gtest
    
    perforator/lib/asm/x86
)

SRCS(
    evaluator_ut.cpp
)

END()
