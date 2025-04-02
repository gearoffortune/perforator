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

TPythonAnalyzer::TPythonAnalyzer(const llvm::object::ObjectFile& file) : File_(file) {}

void TPythonAnalyzer::ParseSymbolLocations() {
    if (Symbols_ != nullptr) {
        return;
    }

    Symbols_ = MakeHolder<TSymbols>();

    auto dynamicSymbols = NELF::RetrieveDynamicSymbols(File_,
        kPyVersionSymbol,
        kPyThreadStateGetCurrentSymbol,
        kPyGetVersionSymbol,
        kPyRuntimeSymbol,
        kPyGILStateCheckSymbol
    );

    auto setSymbolIfFound = [&](const THashMap<TStringBuf, NPerforator::NELF::TSymbolLocation>& symbols, TStringBuf symbolName, TMaybe<NELF::TSymbolLocation>& target) {
        if (auto it = symbols.find(symbolName); it != symbols.end()) {
            target = it->second;
        }
    };

    if (dynamicSymbols) {
        setSymbolIfFound(*dynamicSymbols, kPyVersionSymbol, Symbols_->PyVersion);
        setSymbolIfFound(*dynamicSymbols, kPyThreadStateGetCurrentSymbol, Symbols_->GetCurrentThreadState);
        setSymbolIfFound(*dynamicSymbols, kPyGetVersionSymbol, Symbols_->PyGetVersion);
        setSymbolIfFound(*dynamicSymbols, kPyRuntimeSymbol, Symbols_->PyRuntime);
        setSymbolIfFound(*dynamicSymbols, kPyGILStateCheckSymbol, Symbols_->PyGILStateCheck);
    }

    auto symbols = NELF::RetrieveSymbols(File_, kCurrentFastGetSymbol);
    if (symbols) {
        setSymbolIfFound(*symbols, kCurrentFastGetSymbol, Symbols_->CurrentFastGet);
    }

    return;
}

template <typename ELFT>
TMaybe<TPythonVersion> TryParseVersionFromPyVersionSymbol(
    const llvm::object::SectionRef& section,
    llvm::StringRef sectionData,
    const NPerforator::NELF::TSymbolLocation& pyVersion
) {
    if (pyVersion.Address < section.getAddress()) {
        return Nothing();
    }

    if (pyVersion.Size != sizeof(ui32) && pyVersion.Size != sizeof(ui64)) {
        return Nothing();
    }

    ui64 offset = pyVersion.Address - section.getAddress();
    if (offset + pyVersion.Size >= sectionData.size()) {
        return Nothing();
    }

    TStringBuf versionView{sectionData.data() + offset, pyVersion.Size};
    TVector<char> versionBytes(versionView.begin(), versionView.end());
    if constexpr (ELFT::TargetEndianness == llvm::endianness::little) {
        Reverse(versionBytes.begin(), versionBytes.end());
    }

    ui32 skipBytes = (versionBytes.size() == sizeof(ui64)) ? sizeof(ui32) : 0;

    return MakeMaybe(TPythonVersion{
        .MajorVersion = ui8(versionBytes[skipBytes + 0]),
        .MinorVersion = ui8(versionBytes[skipBytes + 1]),
        .MicroVersion = ui8(versionBytes[skipBytes + 2]),
    });
}

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

TMaybe<TPythonVersion> TryParsePyGetVersion(
    const llvm::object::ObjectFile& file,
    const NPerforator::NELF::TSymbolLocation& pyGetVersion
) {
    auto textSection = NELF::GetSection(file, NELF::NSections::kTextSectionName);
    if (!textSection) {
        return Nothing();
    }

    Y_LLVM_UNWRAP(sectionData, textSection->getContents(), { return Nothing(); });
    if (pyGetVersion.Address < textSection->getAddress()) {
        return Nothing();
    }

    ui64 offset = pyGetVersion.Address - textSection->getAddress();
    if (offset >= sectionData.size()) {
        return Nothing();
    }

    TConstArrayRef<ui8> bytecode(
        reinterpret_cast<const ui8*>(sectionData.data()) + offset,
        Min<size_t>(64, sectionData.size() - offset)  // Limit to first 64 bytes
    );

    auto versionAddress = NAsm::NX86::DecodePyGetVersion(file.makeTriple(), pyGetVersion.Address, bytecode);
    if (!versionAddress) {
        return Nothing();
    }

    auto rodataSection = NELF::GetSection(file, NELF::NSections::kRoDataSectionName);
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

    return TryScanVersion(TConstArrayRef<char>(
        rodataData.data() + versionOffset,
        Min<size_t>(10, rodataData.size() - versionOffset)  // Limit to 10 bytes which is enough for "X.YY.ZZZ"
    ));
}

template <typename ELFT>
TMaybe<TParsedPythonVersion> ParseVersion(
    const llvm::object::ObjectFile& file,
    const TPythonAnalyzer::TSymbols& symbols
) {
    const llvm::object::ELFObjectFile<ELFT>* elf = llvm::dyn_cast<llvm::object::ELFObjectFile<ELFT>>(&file);
    if (!elf) {
        return Nothing();
    }

    // First try Py_Version symbol if available
    if (symbols.PyVersion) {
        auto section = NELF::GetSection(*elf, NELF::NSections::kRoDataSectionName);
        if (section) {
            Y_LLVM_UNWRAP(sectionData, section->getContents(), { return Nothing(); });
            if (auto version = TryParseVersionFromPyVersionSymbol<ELFT>(*section, sectionData, *symbols.PyVersion)) {
                return MakeMaybe(TParsedPythonVersion{
                    .Version = *version,
                    .Source = EPythonVersionSource::PyVersionSymbol
                });
            }
        }
    }

    // Try to find PY_VERSION string through Py_GetVersion disassembly
    if (symbols.PyGetVersion) {
        if (auto version = TryParsePyGetVersion(*elf, *symbols.PyGetVersion)) {
            return MakeMaybe(TParsedPythonVersion{
                .Version = *version,
                .Source = EPythonVersionSource::PyGetVersionDisassembly
            });
        }
    }

    return Nothing();
}

TMaybe<TParsedPythonVersion> TPythonAnalyzer::ParseVersion() {
    ParseSymbolLocations();

    #define TRY_ELF_TYPE(ELFT) \
    if (auto res = NPerforator::NLinguist::NPython::ParseVersion<ELFT>(File_, *Symbols_.Get())) { \
        return res; \
    }

    Y_LLVM_FOR_EACH_ELF_TYPE(TRY_ELF_TYPE)

#undef TRY_ELF_TYPE
    return Nothing();
}

TMaybe<NAsm::ThreadImageOffsetType> TPythonAnalyzer::ParseTLSPyThreadState() {
    if (!Symbols_) {
        ParseSymbolLocations();
    }

    if (File_.getArch() != llvm::Triple::x86 && File_.getArch() != llvm::Triple::x86_64) {
        return Nothing();
    }

    if (!Symbols_->GetCurrentThreadState) {
        return Nothing();
    }

    // current_fast_get might not be inlined into GetCurrentThreadState, so we should disassemble it instead of PyThreadState_GetCurrent.
    const NPerforator::NELF::TSymbolLocation& getter = Symbols_->CurrentFastGet ? *Symbols_->CurrentFastGet : *Symbols_->GetCurrentThreadState;

    if (getter.Address == 0) {
        return Nothing();
    }

    auto textSection = NELF::GetSection(File_, NELF::NSections::kTextSectionName);
    if (!textSection) {
        return Nothing();
    }

    Y_LLVM_UNWRAP(sectionData, textSection->getContents(), { return Nothing(); });
    if (getter.Address < textSection->getAddress()) {
        return Nothing();
    }

    ui64 offset = getter.Address - textSection->getAddress();
    if (offset > sectionData.size()) {
        return Nothing();
    }

    TConstArrayRef<ui8> bytecode(
        reinterpret_cast<const ui8*>(sectionData.data()) + offset,
        Min(getter.Size, sectionData.size() - offset)
    );

    if (Symbols_->CurrentFastGet) {
        return NAsm::NX86::DecodeCurrentFastGet(File_.makeTriple(), bytecode);
    }

    return NAsm::NX86::DecodePyThreadStateGetCurrent(File_.makeTriple(), bytecode);
}

bool IsPythonBinary(const llvm::object::ObjectFile& file) {
    auto dynamicSymbols = NELF::RetrieveDynamicSymbols(file, kPyGetVersionSymbol);
    return (dynamicSymbols && !dynamicSymbols->empty());
}

TMaybe<ui64> TPythonAnalyzer::ParsePyRuntimeAddress() {
    ParseSymbolLocations();

    if (!Symbols_) {
        return Nothing();
    }

    if (!Symbols_->PyRuntime || Symbols_->PyRuntime->Address == 0) {
        return Nothing();
    }

    return MakeMaybe(Symbols_->PyRuntime->Address);
}

TMaybe<ui64> TPythonAnalyzer::ParseAutoTSSKeyAddress() {
    if (File_.getArch() != llvm::Triple::x86 && File_.getArch() != llvm::Triple::x86_64) {
        return Nothing();
    }

    if (!Symbols_ || Symbols_->PyRuntime->Address == 0) {
        return Nothing();
    }

    if (!Symbols_->PyGILStateCheck || Symbols_->PyGILStateCheck->Address == 0) {
        return Nothing();
    }

    auto textSection = NELF::GetSection(File_, NELF::NSections::kTextSectionName);
    if (!textSection) {
        return Nothing();
    }

    Y_LLVM_UNWRAP(sectionData, textSection->getContents(), { return Nothing(); });
    if (Symbols_->PyGILStateCheck->Address < textSection->getAddress()) {
        return Nothing();
    }

    ui64 offset = Symbols_->PyGILStateCheck->Address - textSection->getAddress();
    if (offset >= sectionData.size()) {
        return Nothing();
    }

    size_t symbolSize = 0;
    if (Symbols_->PyGILStateCheck->Size > 0) {
        symbolSize = Symbols_->PyGILStateCheck->Size;
    } else {
        // 100 bytes should be enough for PyGILState_Check
        symbolSize = 100;
    }

    TConstArrayRef<ui8> bytecode(
        reinterpret_cast<const ui8*>(sectionData.data()) + offset,
        Min<size_t>(symbolSize, sectionData.size() - offset)
    );

    return NAsm::NX86::DecodeAutoTSSKeyAddress(
        File_.makeTriple(),
        Symbols_->PyGILStateCheck->Address,
        bytecode
    );
}

} // namespace NPerforator::NLinguist::NPython
