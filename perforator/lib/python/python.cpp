#include "python.h"

#include <llvm/MC/MCContext.h>
#include <llvm/MC/MCTargetOptions.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Object/ELF.h>
#include <llvm/Object/ELFObjectFile.h>
#include <llvm/Object/ObjectFile.h>

#include <perforator/lib/elf/elf.h>
#include <perforator/lib/llvmex/llvm_elf.h>
#include <perforator/lib/llvmex/llvm_exception.h>

#include <util/generic/adaptor.h>
#include <util/generic/array_ref.h>
#include <util/generic/vector.h>
#include <util/stream/format.h>

#include <contrib/libs/re2/re2/stringpiece.h>
#include <util/string/builder.h>

namespace NPerforator::NLinguist::NPython {

// Some limit for decoding of instructions of `_PyThreadState_GetCurrent` function
constexpr ui64 kMaxPyThreadStateGetCurrentBytecodeLength = 64;

TPythonAnalyzer::TPythonAnalyzer(const llvm::object::ObjectFile& file) : File_(file) {}

void TPythonAnalyzer::ParseSymbolsAddresses() {
    if (SymbolsAddresses_ != nullptr) {
        return;
    }

    SymbolsAddresses_ = MakeHolder<TSymbolsAddresses>();

    auto dynamicSymbols = NELF::RetrieveDynamicSymbols(File_,
        kPyVersionSymbol,
        kPyThreadStateGetCurrentSymbol,
        kPyGetVersionSymbol,
        kPyRuntimeSymbol,
        kPyGILStateCheckSymbol
    );

    if (dynamicSymbols) {
        SymbolsAddresses_->PyVersion = (*dynamicSymbols)[kPyVersionSymbol];
        SymbolsAddresses_->GetCurrentThreadState = (*dynamicSymbols)[kPyThreadStateGetCurrentSymbol];
        SymbolsAddresses_->PyGetVersion = (*dynamicSymbols)[kPyGetVersionSymbol];
        SymbolsAddresses_->PyRuntime = (*dynamicSymbols)[kPyRuntimeSymbol];
        SymbolsAddresses_->PyGILStateCheck = (*dynamicSymbols)[kPyGILStateCheckSymbol];
    }

    auto symbols = NELF::RetrieveSymbols(File_, kCurrentFastGetSymbol);
    if (symbols) {
        SymbolsAddresses_->CurrentFastGet = (*symbols)[kCurrentFastGetSymbol];
    }

    return;
}

template <typename ELFT>
TMaybe<TPythonVersion> TryParseVersionFromPyVersionSymbol(
    const llvm::object::SectionRef& section,
    llvm::StringRef sectionData,
    ui64 pyVersionAddress
) {
    if (pyVersionAddress < section.getAddress()) {
        return Nothing();
    }

    ui64 offset = pyVersionAddress - section.getAddress();
    if (offset + sizeof(ui32) >= sectionData.size()) {
        return Nothing();
    }

    TStringBuf versionView{sectionData.data() + offset, sizeof(ui32)};
    TVector<char> versionBytes(versionView.begin(), versionView.end());
    if constexpr (ELFT::TargetEndianness == llvm::endianness::little) {
        Reverse(versionBytes.begin(), versionBytes.end());
    }

    return MakeMaybe(TPythonVersion{
        .MajorVersion = ui8(versionBytes[0]),
        .MinorVersion = ui8(versionBytes[1]),
        .MicroVersion = ui8(versionBytes[2]),
    });
}

template <typename ELFT>
TMaybe<TPythonVersion> TryScanVersion(
    TConstArrayRef<char> data
) {
    /*
     * Python version string formats:
     * - Python < 3.3.0: Can be either X.Y (e.g. "2.6") or X.Y.Z (e.g. "2.7.17")
     * - Python >= 3.3.0: Always X.Y.Z format (e.g. "3.3.0", "3.12.1")
     */
    re2::StringPiece input(data.data(), data.size());
    std::string major, minor, micro, suffix;


    while (re2::RE2::FindAndConsume(&input, kPythonVersionRegex, &major, &minor, &micro, &suffix)) {
        ui8 majorVersion = static_cast<ui8>(std::stoi(major));
        ui8 minorVersion = static_cast<ui8>(std::stoi(minor));
        ui8 microVersion = micro.empty() ? 0 : static_cast<ui8>(std::stoi(micro));

        // For X.Y format, only accept versions < 3.3.0
        if (micro.empty() && (majorVersion == 3 && minorVersion >= 3)) {
            continue;
        }

        return TPythonVersion{
            .MajorVersion = majorVersion,
            .MinorVersion = minorVersion,
            .MicroVersion = microVersion,
        };
    }

    return Nothing();
}

template <typename ELFT>
TMaybe<TPythonVersion> TryParsePyGetVersion(
    const llvm::object::ELFObjectFile<ELFT>& elf,
    ui64 pyGetVersionAddress
) {
    auto textSection = NELF::GetSection(elf, NELF::NSections::kTextSectionName);
    if (!textSection) {
        return Nothing();
    }

    Y_LLVM_UNWRAP(sectionData, textSection->getContents(), { return Nothing(); });
    if (pyGetVersionAddress < textSection->getAddress()) {
        return Nothing();
    }

    ui64 offset = pyGetVersionAddress - textSection->getAddress();
    if (offset >= sectionData.size()) {
        return Nothing();
    }

    TConstArrayRef<ui8> bytecode(
        reinterpret_cast<const ui8*>(sectionData.data()) + offset,
        Min<size_t>(64, sectionData.size() - offset)  // Limit to first 64 bytes
    );

    auto versionAddress = NAsm::NX86::DecodePyGetVersion(elf.makeTriple(), pyGetVersionAddress, bytecode);
    if (!versionAddress) {
        return Nothing();
    }

    auto rodataSection = NELF::GetSection(elf, NELF::NSections::kRoDataSectionName);
    if (!rodataSection) {
        return Nothing();
    }

    Y_LLVM_UNWRAP(rodataData, rodataSection->getContents(), { return Nothing(); });
    if (*versionAddress < rodataSection->getAddress()) {
        return Nothing();
    }

    ui64 versionOffset = *versionAddress - rodataSection->getAddress();
    if (versionOffset >= rodataData.size()) {
        return Nothing();
    }

    return TryScanVersion<ELFT>(TConstArrayRef<char>(
        rodataData.data() + versionOffset,
        Min<size_t>(10, rodataData.size() - versionOffset)  // Limit to 10 bytes which is enough for "X.YY.ZZZ"
    ));
}

template <typename ELFT>
TMaybe<TParsedPythonVersion> ParseVersion(
    const llvm::object::ObjectFile& file,
    const TPythonAnalyzer::TSymbolsAddresses& addresses
) {
    const llvm::object::ELFObjectFile<ELFT>* elf = llvm::dyn_cast<llvm::object::ELFObjectFile<ELFT>>(&file);
    if (!elf) {
        return Nothing();
    }

    // First try Py_Version symbol if available
    if (addresses.PyVersion != 0) {
        auto section = NELF::GetSection(*elf, NELF::NSections::kRoDataSectionName);
        if (section) {
            Y_LLVM_UNWRAP(sectionData, section->getContents(), { return Nothing(); });
            if (auto version = TryParseVersionFromPyVersionSymbol<ELFT>(*section, sectionData, addresses.PyVersion)) {
                return MakeMaybe(TParsedPythonVersion{
                    .Version = *version,
                    .Source = EPythonVersionSource::PyVersionSymbol
                });
            }
        }
    }

    // Try to find PY_VERSION string through Py_GetVersion disassembly
    if (addresses.PyGetVersion != 0) {
        if (auto version = TryParsePyGetVersion(*elf, addresses.PyGetVersion)) {
            return MakeMaybe(TParsedPythonVersion{
                .Version = *version,
                .Source = EPythonVersionSource::PyGetVersionDisassembly
            });
        }
    }

    return Nothing();
}

TMaybe<TParsedPythonVersion> TPythonAnalyzer::ParseVersion() {
    ParseSymbolsAddresses();

    #define TRY_ELF_TYPE(ELFT) \
    if (auto res = NPerforator::NLinguist::NPython::ParseVersion<ELFT>(File_, *SymbolsAddresses_.Get())) { \
        return res; \
    }

    Y_LLVM_FOR_EACH_ELF_TYPE(TRY_ELF_TYPE)

#undef TRY_ELF_TYPE
    return Nothing();
}

template <typename ELFT>
TMaybe<NAsm::ThreadImageOffsetType> ParseTLSPyThreadState(
    const llvm::object::ObjectFile& file,
    TPythonAnalyzer::TSymbolsAddresses* addresses
) {
    const llvm::object::ELFObjectFile<ELFT>* elf = llvm::dyn_cast<llvm::object::ELFObjectFile<ELFT>>(&file);
    if (!elf) {
        return Nothing();
    }

    if (elf->getArch() != llvm::Triple::x86 && elf->getArch() != llvm::Triple::x86_64) {
        return Nothing();
    }

    if (addresses->GetCurrentThreadState == 0) {
        return Nothing();
    }

    // current_fast_get might not be inlined into GetCurrentThreadState, so we should disassemble it instead of PyThreadState_GetCurrent.
    ui64 getterAddress = (addresses->CurrentFastGet != 0) ? addresses->CurrentFastGet : addresses->GetCurrentThreadState;

    auto textSection = NELF::GetSection(*elf, NELF::NSections::kTextSectionName);
    if (!textSection) {
        return Nothing();
    }

    Y_LLVM_UNWRAP(sectionData, textSection->getContents(), { return Nothing(); });
    if (getterAddress < textSection->getAddress()) {
        return Nothing();
    }

    ui64 offset = getterAddress - textSection->getAddress();
    if (offset > sectionData.size()) {
        return Nothing();
    }

    TConstArrayRef<ui8> bytecode(
        reinterpret_cast<const ui8*>(sectionData.data()) + offset,
        Min(kMaxPyThreadStateGetCurrentBytecodeLength, sectionData.size() - offset)
    );

    if (addresses->CurrentFastGet != 0) {
        return NAsm::NX86::DecodeCurrentFastGet(elf->makeTriple(), bytecode);
    }

    return NAsm::NX86::DecodePyThreadStateGetCurrent(elf->makeTriple(), bytecode);
}

TMaybe<NAsm::ThreadImageOffsetType> TPythonAnalyzer::ParseTLSPyThreadState() {
    ParseSymbolsAddresses();

    #define TRY_ELF_TYPE(ELFT) \
    if (auto res = NPerforator::NLinguist::NPython::ParseTLSPyThreadState<ELFT>(File_, SymbolsAddresses_.Get())) { \
        return res; \
    }

    Y_LLVM_FOR_EACH_ELF_TYPE(TRY_ELF_TYPE)

#undef TRY_ELF_TYPE
    return Nothing();
}

bool IsPythonBinary(const llvm::object::ObjectFile& file) {
    auto dynamicSymbols = NELF::RetrieveDynamicSymbols(file, kPyGetVersionSymbol);
    return (dynamicSymbols && !dynamicSymbols->empty());
}

TMaybe<ui64> TPythonAnalyzer::ParsePyRuntimeAddress() {
    ParseSymbolsAddresses();

    if (!SymbolsAddresses_) {
        return Nothing();
    }

    if (SymbolsAddresses_->PyRuntime == 0) {
        return Nothing();
    }

    return MakeMaybe(SymbolsAddresses_->PyRuntime);
}

template <typename ELFT>
TMaybe<ui64> ParseAutoTSSKeyAddress(
    const llvm::object::ObjectFile& file,
    const TPythonAnalyzer::TSymbolsAddresses& addresses
) {
    const llvm::object::ELFObjectFile<ELFT>* elf = llvm::dyn_cast<llvm::object::ELFObjectFile<ELFT>>(&file);
    if (!elf) {
        return Nothing();
    }

    if (addresses.PyRuntime == 0) {
        return Nothing();
    }

    if (addresses.PyGILStateCheck == 0) {
        return Nothing();
    }

    auto textSection = NELF::GetSection(*elf, NELF::NSections::kTextSectionName);
    if (!textSection) {
        return Nothing();
    }

    Y_LLVM_UNWRAP(sectionData, textSection->getContents(), { return Nothing(); });
    if (addresses.PyGILStateCheck < textSection->getAddress()) {
        return Nothing();
    }

    ui64 offset = addresses.PyGILStateCheck - textSection->getAddress();
    if (offset >= sectionData.size()) {
        return Nothing();
    }

    // Disassemble at most 100 bytes, which should be enough for PyGILState_Check
    TConstArrayRef<ui8> bytecode(
        reinterpret_cast<const ui8*>(sectionData.data()) + offset,
        Min<size_t>(100, sectionData.size() - offset)
    );

    return NAsm::NX86::DecodeAutoTSSKeyAddress(
        elf->makeTriple(),
        addresses.PyGILStateCheck,
        bytecode
    );
}

TMaybe<ui64> TPythonAnalyzer::ParseAutoTSSKeyAddress() {
    ParseSymbolsAddresses();

    if (!SymbolsAddresses_) {
        return Nothing();
    }

    #define TRY_ELF_TYPE(ELFT) \
    if (auto res = NPerforator::NLinguist::NPython::ParseAutoTSSKeyAddress<ELFT>(File_, *SymbolsAddresses_.Get())) { \
        return res; \
    }

    Y_LLVM_FOR_EACH_ELF_TYPE(TRY_ELF_TYPE)

#undef TRY_ELF_TYPE
    return Nothing();
}

} // namespace NPerforator::NLinguist::NPython
