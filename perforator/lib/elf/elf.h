#pragma once

#include <util/generic/string.h>
#include <util/generic/hash.h>
#include <util/generic/vector.h>
#include <util/generic/maybe.h>

#include <llvm/Object/ObjectFile.h>
#include <llvm/Object/ELFObjectFile.h>

namespace NPerforator::NELF {

struct TSymbolLocation {
    ui64 Address = 0;
    ui64 Size = 0;
};

namespace NSections {

constexpr TStringBuf kTextSectionName = ".text";
constexpr TStringBuf kDataSectionName = ".data";
constexpr TStringBuf kBssSectionName = ".bss";
constexpr TStringBuf kRoDataSectionName = ".rodata";

} // namespace NPerforator::NELF::NSections

namespace NPrivate {

TMaybe<THashMap<TStringBuf, TSymbolLocation>> RetrieveDynamicSymbols(const llvm::object::ObjectFile& file, std::initializer_list<TStringBuf> symbols);

TMaybe<THashMap<TStringBuf, TSymbolLocation>> RetrieveSymbols(const llvm::object::ObjectFile& file, std::initializer_list<TStringBuf> symbols);

} // namespace NPerforator::NELF::NPrivate

template <typename... Args>
TMaybe<THashMap<TStringBuf, TSymbolLocation>> RetrieveDynamicSymbols(const llvm::object::ObjectFile& file, Args... symbols) {
    return NPerforator::NELF::NPrivate::RetrieveDynamicSymbols(file, {symbols...});
}

template <typename... Args>
TMaybe<THashMap<TStringBuf, TSymbolLocation>> RetrieveSymbols(const llvm::object::ObjectFile& file, Args... symbols) {
    return NPerforator::NELF::NPrivate::RetrieveSymbols(file, {symbols...});
}

TMaybe<llvm::object::SectionRef> GetSection(const llvm::object::ObjectFile& file, TStringBuf sectionName);

} // namespace NPerforator::NELF
