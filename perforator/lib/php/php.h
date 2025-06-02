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

enum class EZendVmKind {
    Call,
    Switch,
    Goto,
    Hybrid,
};

TString ToString(const EZendVmKind vmKind);

std::strong_ordering operator<=>(const TPhpVersion& lhs, const TPhpVersion& rhs);

constexpr TStringBuf kPhpVersionSymbol = "php_version";
constexpr TStringBuf kZmInfoPhpCoreSymbol = "zm_info_php_core";
constexpr TStringBuf kZendVmKindSymbol = "zend_vm_kind";
constexpr TStringBuf kPhpTsrmStartupSymbol = "php_tsrm_startup";
constexpr TStringBuf kExecutorGlobalsSymbol = "executor_globals";

// since:   PHP-4.0
// link:    https://github.com/php/php-src/blob/b871261c10fcf5ffef3851ae31ac12a0170044d2/main/SAPI.h#L309
// section: rodata
constexpr TStringBuf kPhpVersionKeyPhrase = "X-Powered-By: PHP/";
const re2::RE2 kPhpVersionRegex(R"((\d)\.(\d)\.(\d+))");

class TZendPhpAnalyzer {
public:
    struct TSymbols {
        // since:   PHP-8.3.0
        // link:    https://github.com/php/php-src/blob/f64e3d5d9ebd72f5d812136ac612fae435c620a0/main/main.c#L103
        // section: dynsym
        TMaybe<NPerforator::NELF::TLocation> PhpVersion;

        // since:   PHP-5.3.11
        // link:    https://github.com/php/php-src/blob/f64e3d5d9ebd72f5d812136ac612fae435c620a0/main/main.c#L2064
        // section: symtab
        TMaybe<NPerforator::NELF::TLocation> ZmInfoPhpCore;

        // since:   PHP-7.2.0
        // link:    https://github.com/php/php-src/blob/b871261c10fcf5ffef3851ae31ac12a0170044d2/Zend/zend_vm_execute.skl#L164
        // section: dynsym
        TMaybe<NPerforator::NELF::TLocation> ZendVmKind;

        // since:   PHP-7.4.0
        // link:    https://github.com/php/php-src/blob/b871261c10fcf5ffef3851ae31ac12a0170044d2/main/php_main.h#L78
        // section: dynsym
        TMaybe<NPerforator::NELF::TLocation> PhpTsrmStartup;

        // since:   PHP-5.0
        // link:    https://github.com/php/php-src/blob/b871261c10fcf5ffef3851ae31ac12a0170044d2/Zend/zend_globals_macros.h#L47
        // section: dynsym
        TMaybe<NPerforator::NELF::TLocation> ExecutorGlobals;
    };

public:
    explicit TZendPhpAnalyzer(const llvm::object::ObjectFile& file);

    TMaybe<TParsedPhpVersion> ParseVersion();

    TMaybe<EZendVmKind> ParseZendVmKind();

    TMaybe<bool> ParseZts();

    TMaybe<ui64> ParseExecutorGlobals();

private:
    void ParseSymbolLocations();

private:
    const llvm::object::ObjectFile& File_;
    THolder<TSymbols> Symbols_;
    TMaybe<TParsedPhpVersion> Version_ = Nothing();
};

} // namespace NPerforator::NLinguist::NPhp
