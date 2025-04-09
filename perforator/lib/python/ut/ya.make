GTEST()

ADDINCL(
    ${ARCADIA_BUILD_ROOT}/contrib/libs/llvm18/lib/Target/X86
)

PEERDIR(
    contrib/libs/re2

    library/cpp/logger/global

    perforator/lib/python
)

SRCS(
    parse_version_ut.cpp
)

END()
