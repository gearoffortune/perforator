UNITTEST()

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
    
    perforator/lib/asm/x86
    library/cpp/logger/global
)

SRCS(
    evaluator_ut.cpp
)

END()
