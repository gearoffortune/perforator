#include <llvm/Object/ObjectFile.h>
#include <llvm/Support/TargetSelect.h>

#include <perforator/lib/pthread/pthread.h>
#include <perforator/lib/llvmex/llvm_exception.h>

#include <util/stream/format.h>
#include <util/stream/output.h>


int main(int argc, const char* argv[]) {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetDisassembler();

    Y_THROW_UNLESS(argc == 2);
    auto objectFile = Y_LLVM_RAISE(llvm::object::ObjectFile::createObjectFile(argv[1]));

    if (!NPerforator::NPthread::IsLibPthreadBinary(*objectFile.getBinary())) {
        Cout << "Does not seem like pthread library" << Endl;
        return 0;
    }

    Cout << "Detected pthread library" << Endl;

    NPerforator::NPthread::TLibPthreadAnalyzer analyzer{*objectFile.getBinary()};

    auto tssInfo = analyzer.ParseAccessTSSInfo();
    if (!tssInfo) {
        Cout << "Could not parse pthread TLS structure information" << Endl;
        return 0;
    }

    Cout << "Pthread TSS information:" << Endl;
    Cout << "  PTHREAD_KEY_2NDLEVEL_SIZE: " << tssInfo->KeySecondLevelSize << Endl;
    Cout << "  PTHREAD_KEY_1STLEVEL_SIZE: " << tssInfo->KeyFirstLevelSize << Endl;
    Cout << "  PTHREAD_KEYS_MAX: " << tssInfo->KeysMax << Endl;

    Cout << "  pthread_key_data struct:" << Endl;
    Cout << "    Size: " << tssInfo->PthreadKeyData.Size << " bytes" << Endl;
    Cout << "    Value offset: " << tssInfo->PthreadKeyData.ValueOffset << Endl;
    Cout << "    Seq offset: " << tssInfo->PthreadKeyData.SeqOffset << Endl;

    Cout << "  pthread struct fields:" << Endl;
    Cout << "    specific_1stblock offset: " << tssInfo->FirstSpecificBlockOffset << Endl;
    Cout << "    specific array offset: " << tssInfo->SpecificArrayOffset << Endl;

    if (tssInfo->StructPthreadPointerOffset) {
        Cout << "  struct pthread pointer offset: " << tssInfo->StructPthreadPointerOffset << Endl;
    } else {
        Cout << "  struct pthread pointer offset not found" << Endl;
    }

    return 0;
}
