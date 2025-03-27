#include "elf.h"

#include <perforator/lib/llvmex/llvm_elf.h>
#include <perforator/lib/llvmex/llvm_exception.h>

#include <llvm/Object/ELFObjectFile.h>

namespace NPerforator::NELF::NPrivate {

THashMap<TStringBuf, ui64> ParseSymbolsImpl(
    const llvm::object::ELFObjectFileBase::elf_symbol_iterator_range& symbols,
    std::initializer_list<TStringBuf> targetSymbols
) {
    THashMap<TStringBuf, ui64> result;

    for (const auto& symbol : symbols) {
        Y_LLVM_UNWRAP(name, symbol.getName(), { continue; });
        Y_LLVM_UNWRAP(address, symbol.getAddress(), { continue; });

        TStringBuf symbolName{name.data(), name.size()};
        for (const auto& targetSymbol : targetSymbols) {
            if (symbolName == targetSymbol) {
                result[symbolName] = address;
                break;
            }
        }
    }

    return result;
}

template <typename ELFT>
THashMap<TStringBuf, ui64> ParseDynamicSymbols(const llvm::object::ELFObjectFile<ELFT>& elf, std::initializer_list<TStringBuf> symbols) {
    return ParseSymbolsImpl(elf.getDynamicSymbolIterators(), symbols);
}

template <typename ELFT>
THashMap<TStringBuf, ui64> ParseSymbols(const llvm::object::ELFObjectFile<ELFT>& elf, std::initializer_list<TStringBuf> symbols) {
    return ParseSymbolsImpl(elf.symbols(), symbols);
}

TMaybe<THashMap<TStringBuf, ui64>> RetrieveDynamicSymbols(llvm::object::ObjectFile* file, std::initializer_list<TStringBuf> symbols) {
    return NLLVM::VisitELF(file, [&symbols](const auto& elf) {
        return ParseDynamicSymbols(elf, symbols);
    });
}

TMaybe<THashMap<TStringBuf, ui64>> RetrieveSymbols(llvm::object::ObjectFile* file, std::initializer_list<TStringBuf> symbols) {
    return NLLVM::VisitELF(file, [&symbols](const auto& elf) {
        return ParseSymbols(elf, symbols);
    });
}

} // namespace NPerforator::NELF::NPrivate

namespace NPerforator::NELF {

TMaybe<llvm::object::SectionRef> GetSection(const llvm::object::ObjectFile& file, TStringBuf sectionName) {
    for (const auto& section : file.sections()) {
        Y_LLVM_UNWRAP(name, section.getName(), { continue; });
        if (TStringBuf{name.data(), name.size()} == sectionName) {
            return section;
        }
    }

    return Nothing();
}

} // namespace NPerforator::NELF
