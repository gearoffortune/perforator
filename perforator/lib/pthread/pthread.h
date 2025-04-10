#pragma once

#include <llvm/Object/ObjectFile.h>

#include <perforator/lib/elf/elf.h>

#include <util/generic/fwd.h>
#include <util/generic/maybe.h>
#include <util/generic/string.h>
#include <util/string/builder.h>

namespace NPerforator::NPthread {

constexpr TStringBuf kPthreadGetspecificSymbol = "pthread_getspecific";
constexpr TStringBuf kPthreadSetspecificSymbol = "pthread_setspecific";

struct TPthreadKeyDataInfo {
    ui64 Size = 0;
    ui64 ValueOffset = 0;
    ui64 SeqOffset = 0;
};

struct TAccessTSSInfo {
    // TODO: support ui64 TidOffset = 0;
    TPthreadKeyDataInfo PthreadKeyData;
    ui64 FirstSpecificBlockOffset = 0;
    ui64 SpecificArrayOffset = 0;
    ui64 StructPthreadPointerOffset = 0;
    ui64 KeySecondLevelSize = 0;  // PTHREAD_KEY_2NDLEVEL_SIZE
    ui64 KeyFirstLevelSize = 0;   // PTHREAD_KEY_1STLEVEL_SIZE
    ui64 KeysMax = 0;             // PTHREAD_KEYS_MAX
};

class TLibPthreadAnalyzer {
public:
    struct TSymbols {
        TMaybe<NPerforator::NELF::TLocation> PthreadGetspecific;
    };

public:
    explicit TLibPthreadAnalyzer(const llvm::object::ObjectFile& file);

    TMaybe<TAccessTSSInfo> ParseAccessTSSInfo();

private:
    void ParseSymbolLocations();

private:
    const llvm::object::ObjectFile& File_;
    THolder<TSymbols> Symbols_;
};

bool IsLibPthreadBinary(const llvm::object::ObjectFile& file);

} // namespace NPerforator::NPthread
