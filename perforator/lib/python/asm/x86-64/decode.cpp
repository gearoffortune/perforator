#include "decode.h"

namespace NPerforator::NLinguist::NPython::NAsm::NX86 {

static TMaybe<ui64> GetRegisterValueOrAddress(const TState& state, unsigned int reg) {
    if (!state.HasKnownValue(reg)) {
        return Nothing();
    }

    auto immValue = state.GetImmediateValue(reg);
    if (immValue) {
        return static_cast<ui64>(*immValue);
    }

    auto memAddr = state.GetMemoryAddress(reg);
    if (memAddr) {
        return static_cast<ui64>(*memAddr);
    }

    return Nothing();
}

/*
 * Disassembles PyGILState_Check function to find the absolute address of autoTSSkey.
 * Example of PyGILState_Check assembly:

0000000000296e50 <PyGILState_Check>:
  296e50:       f3 0f 1e fa             endbr64
  296e54:       8b 05 5e cd 2d 00       mov    0x2dcd5e(%rip),%eax        # 573bb8 <_PyRuntime+0x238>
  296e5a:       53                      push   %rbx
  296e5b:       85 c0                   test   %eax,%eax
  296e5d:       0f 84 34 b0 ef ff       je     191e97 <PyGILState_Check.cold>
  296e63:       48 8d 3d 66 cd 2d 00    lea    0x2dcd66(%rip),%rdi        # 573bd0 <_PyRuntime+0x250>
  296e6a:       e8 b1 49 00 00          callq  29b820 <PyThread_tss_is_created>
  296e6f:       85 c0                   test   %eax,%eax
  296e71:       0f 84 20 b0 ef ff       je     191e97 <PyGILState_Check.cold>
  296e77:       48 8b 1d 42 cd 2d 00    mov    0x2dcd42(%rip),%rbx        # 573bc0 <_PyRuntime+0x240>
  296e7e:       48 85 db                test   %rbx,%rbx
  296e81:       0f 84 21 b0 ef ff       je     191ea8 <PyGILState_Check.cold+0x11>
  296e87:       48 83 3d 39 cd 2d 00    cmpq   $0x0,0x2dcd39(%rip)        # 573bc8 <_PyRuntime+0x248>
  296e8e:       00
  296e8f:       0f 84 0c b0 ef ff       je     191ea1 <PyGILState_Check.cold+0xa>
  296e95:       48 8d 3d 34 cd 2d 00    lea    0x2dcd34(%rip),%rdi        # 573bd0 <_PyRuntime+0x250>
  296e9c:       e8 4f 49 00 00          callq  29b7f0 <PyThread_tss_get>
 */
TMaybe<ui64> DecodeAutoTSSKeyAddress(
    llvm::object::ObjectFile* elf,
    ui64 functionAddress,
    TConstArrayRef<ui8> bytecode
) {
    auto instructionEvaluator = MakeDefaultInstructionEvaluator();
    TBytecodeEvaluator evaluator(elf->makeTriple(), MakeInitialState(functionAddress), bytecode, *instructionEvaluator, MakeStopOnPassControlFlowCondition());

    auto result = evaluator.Evaluate();

    if (!result.has_value()) {
        return Nothing();
    }

    if (result.has_value()) {
        auto rdiValue = GetRegisterValueOrAddress(result->State, llvm::X86::RDI);
        if (rdiValue) {
            return rdiValue;
        }

        auto ediValue = GetRegisterValueOrAddress(result->State, llvm::X86::EDI);
        if (ediValue) {
            return ediValue;
        }
    }

    return Nothing();
}

} // namespace NPerforator::NLinguist::NPython::NAsm::NX86
