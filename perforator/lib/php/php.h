#pragma once

#include <contrib/libs/re2/re2/re2.h>
#include <perforator/lib/elf/elf.h>
#include <llvm/Object/ObjectFile.h>

#include <util/generic/maybe.h>
#include <util/string/builder.h>
#include <util/system/types.h>

namespace NPerforator::NLinguist::NPhp {

struct TPhpVersion {
    ui8 MajorVersion;
    ui8 MinorVersion;
    ui8 ReleaseVersion;
};

enum class EPhpVersionSource {
    PhpVersionDissasembly,
    ZmInfoPhpCoreDissasembly,
    RodataScan
};

struct TParsedPhpVersion {
    TPhpVersion Version;
    EPhpVersionSource Source;

    TString ToString() const;
};

constexpr TStringBuf kPhpVersionSymbol = "php_version";
constexpr TStringBuf kZmInfoPhpCoreSymbol = "zm_info_php_core";
constexpr TStringBuf kPhpVersionKeyPhrase = "X-Powered-By: PHP/";
const re2::RE2 kPhpVersionRegex(R"((\d)\.(\d)\.(\d+))");

class TZendPhpAnalyzer {
public:
    struct TSymbols {
        TMaybe<NPerforator::NELF::TLocation> PhpVersion;
        TMaybe<NPerforator::NELF::TLocation> ZmInfoPhpCore;
        TMaybe<NPerforator::NELF::TLocation> ExecuteEx;
    };

public:
    explicit TZendPhpAnalyzer(const llvm::object::ObjectFile& file);

    TMaybe<TParsedPhpVersion> ParseVersion();

private:
    void ParseSymbolLocations();

private:
    const llvm::object::ObjectFile& File_;
    THolder<TSymbols> Symbols_;
};

} // namespace NPerforator::NLinguist::NPhp
