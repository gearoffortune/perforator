#pragma once

#include <perforator/lib/python/asm/x86-64/decode.h>

#include <perforator/lib/tls/parser/tls.h>

#include <llvm/Object/ObjectFile.h>

#include <util/generic/maybe.h>
#include <util/generic/string.h>
#include <util/string/builder.h>

#include <contrib/libs/re2/re2/re2.h>

namespace NPerforator::NLinguist::NPython {

constexpr TStringBuf kCurrentFastGetSymbol = "current_fast_get";
constexpr TStringBuf kPyThreadStateGetCurrentSymbol = "_PyThreadState_GetCurrent";
constexpr TStringBuf kPyVersionSymbol = "Py_Version";
constexpr TStringBuf kPyGetVersionSymbol = "Py_GetVersion";
constexpr TStringBuf kRoDataSectionName = ".rodata";
constexpr TStringBuf kTextSectionName = ".text";
constexpr TStringBuf kPyRuntimeSymbol = "_PyRuntime";

const re2::RE2 kPythonVersionRegex(R"(([23])\.(\d)(?:\.(\d{1,2}))?([^\.]|$))");

struct TPythonVersion {
    ui8 MajorVersion = 0;
    ui8 MinorVersion = 0;
    ui8 MicroVersion = 0;
};

enum class EPythonVersionSource {
    PyVersionSymbol,
    PyGetVersionDisassembly
};

struct TParsedPythonVersion {
    TPythonVersion Version;
    EPythonVersionSource Source;

    TString ToString() const {
        TStringBuilder builder;
        builder << ui64(Version.MajorVersion) << "." << ui64(Version.MinorVersion) << "." << ui64(Version.MicroVersion)    ;
        builder << " (source: " << (Source == EPythonVersionSource::PyVersionSymbol ? "Py_Version symbol" : "Py_GetVersion disassembly") << ")";
        return builder;
    }
};

class TPythonAnalyzer {
public:
    struct TGlobalsAddresses {
        ui64 GetCurrentThreadStateAddress = 0;
        ui64 CurrentFastGetAddress = 0;
        ui64 PyVersionAddress = 0;
        ui64 PyGetVersionAddress = 0;
        ui64 PyRuntimeAddress = 0;
    };

public:
    explicit TPythonAnalyzer(llvm::object::ObjectFile* file);

    TMaybe<TParsedPythonVersion> ParseVersion();

    // _Py_tss_tstate (https://github.com/python/cpython/blob/main/Include/internal/pycore_pystate.h#L116)
    TMaybe<NAsm::ThreadImageOffsetType> ParseTLSPyThreadState();

    // _PyRuntime singleton
    TMaybe<ui64> ParsePyRuntimeAddress();

    // Parses the absolute address of autoTSSkey field by disassembling PyGILState_Check
    // In previous versions is known as autoTLSkey static variable
    TMaybe<ui64> ParseAutoTSSKeyAddress();

private:
    void ParseGlobalsAddresses();

private:
    llvm::object::ObjectFile* File_;
    THolder<TGlobalsAddresses> GlobalsAddresses_;
};

bool IsPythonBinary(llvm::object::ObjectFile* file);

} // namespace NPerforator::NLinguist::NPython
