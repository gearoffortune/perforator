#include <perforator/lib/asm/x86/evaluator.h>

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include <contrib/libs/llvm18/lib/Target/X86/X86InstrInfo.h>
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#include <llvm/Support/TargetSelect.h>
#include <llvm/MC/MCInst.h>
#include <llvm/MC/MCRegisterInfo.h>
#include <llvm/Object/ObjectFile.h>
#include <llvm/MC/MCInstBuilder.h>

#include <library/cpp/testing/unittest/registar.h>
#include <library/cpp/logger/global/global.h>

#include <util/generic/vector.h>
#include <util/string/builder.h>

using namespace NPerforator::NLinguist::NPython::NAsm::NX86;

class TEvaluatorTest : public TTestBase {
    UNIT_TEST_SUITE(TEvaluatorTest);
    UNIT_TEST(TestStateBasics);
    UNIT_TEST(TestStopConditionFunctions);
    UNIT_TEST(TestBytecodeEvaluator);
    UNIT_TEST(TestDefaultEvaluator);
    UNIT_TEST_SUITE_END();

private:
    void TestStateBasics();
    void TestStopConditionFunctions();
    void TestBytecodeEvaluator();
    void TestDefaultEvaluator();
};

UNIT_TEST_SUITE_REGISTRATION(TEvaluatorTest);

void TEvaluatorTest::TestStateBasics() {
    TState state;
    const unsigned int testReg1 = 123;
    const unsigned int testReg2 = 456;

    UNIT_ASSERT(!state.HasKnownValue(testReg1));
    UNIT_ASSERT(!state.GetImmediateValue(testReg1).Defined());
    UNIT_ASSERT(!state.GetMemoryAddress(testReg1).Defined());

    const i64 immediateValue = 0x12345678;
    state.SetImmediate(testReg1, immediateValue);
    UNIT_ASSERT(state.HasKnownValue(testReg1));
    UNIT_ASSERT(state.GetImmediateValue(testReg1).Defined());
    UNIT_ASSERT_EQUAL(*state.GetImmediateValue(testReg1), immediateValue);
    UNIT_ASSERT(!state.GetMemoryAddress(testReg1).Defined());

    const i64 memoryAddress = 0xABCDEF;
    state.SetMemoryRef(testReg2, memoryAddress);
    UNIT_ASSERT(state.HasKnownValue(testReg2));
    UNIT_ASSERT(!state.GetImmediateValue(testReg2).Defined());
    UNIT_ASSERT(state.GetMemoryAddress(testReg2).Defined());
    UNIT_ASSERT_EQUAL(*state.GetMemoryAddress(testReg2), memoryAddress);

    state.SetImmediate(testReg2, immediateValue);
    UNIT_ASSERT(state.GetImmediateValue(testReg2).Defined());
    UNIT_ASSERT(!state.GetMemoryAddress(testReg2).Defined());
    UNIT_ASSERT_EQUAL(*state.GetImmediateValue(testReg2), immediateValue);
}

void TEvaluatorTest::TestStopConditionFunctions() {
    const unsigned int initialRIP = 0x1000;
    TState initialState = MakeInitialState(initialRIP);

    auto cfStopCondition = MakeStopOnPassControlFlowCondition();

    llvm::MCInst callInst = llvm::MCInstBuilder(llvm::X86::CALLpcrel32).addImm(0);

    UNIT_ASSERT(IsJumpOrCall(callInst));
    UNIT_ASSERT(cfStopCondition(initialState, callInst));

    llvm::MCInst movInst = llvm::MCInstBuilder(llvm::X86::MOV32ri).addReg(llvm::X86::EAX).addImm(1);
    UNIT_ASSERT(!IsJumpOrCall(movInst));
    UNIT_ASSERT(!cfStopCondition(initialState, movInst));
}

void TEvaluatorTest::TestBytecodeEvaluator() {
    const unsigned int initialRIP = 0x1000;
    TState initialState = MakeInitialState(initialRIP);

    auto defaultEvaluator = MakeDefaultInstructionEvaluator();
    UNIT_ASSERT(defaultEvaluator != nullptr);

    // Using a sequence of x86 instructions with both MOV and LEA:
    // 1. MOV EAX, 42              (B8 2A 00 00 00)
    // 2. MOV EBX, 10              (BB 0A 00 00 00)
    // 3. LEA ECX, [EAX+10]        (8D 48 0A)
    // 4. LEA EDX, [ECX+EBX*2]     (8D 54 59 00)

    TVector<ui8> bytecode = {
        0xB8, 0x2A, 0x00, 0x00, 0x00,  // MOV EAX, 42
        0xBB, 0x0A, 0x00, 0x00, 0x00,  // MOV EBX, 10
        0x8D, 0x48, 0x0A,              // LEA ECX, [EAX+10]
        0x8D, 0x54, 0x59, 0x00         // LEA EDX, [ECX+EBX*2]
    };

    llvm::Triple triple("x86_64-unknown-linux-gnu");
    auto cfStopCondition = MakeStopOnPassControlFlowCondition();

    TBytecodeEvaluator evaluator(
        triple,
        initialState,
        TConstArrayRef<ui8>(bytecode.data(), bytecode.size()),
        *defaultEvaluator,
        cfStopCondition
    );

    auto result = evaluator.Evaluate();
    UNIT_ASSERT(result.has_value());

    auto raxValue = result->State.GetImmediateValue(llvm::X86::RAX);
    UNIT_ASSERT(raxValue.Defined());
    UNIT_ASSERT_EQUAL(*raxValue, 42);

    auto rbxValue = result->State.GetImmediateValue(llvm::X86::RBX);
    UNIT_ASSERT(rbxValue.Defined());
    UNIT_ASSERT_EQUAL(*rbxValue, 10);

    auto rcxValue = result->State.GetImmediateValue(llvm::X86::RCX);
    UNIT_ASSERT(rcxValue.Defined());
    UNIT_ASSERT_EQUAL(*rcxValue, 52);  // RAX(42) + 10 = 52, upper 32 bits zero

    auto rdxValue = result->State.GetImmediateValue(llvm::X86::RDX);
    UNIT_ASSERT(rdxValue.Defined());
    UNIT_ASSERT_EQUAL(*rdxValue, 72);  // RCX(52) + RBX(10)*2 = 72, upper 32 bits zero

    auto ripValue = result->State.GetImmediateValue(llvm::X86::RIP);
    UNIT_ASSERT(ripValue.Defined());
    UNIT_ASSERT_EQUAL(*ripValue, initialRIP + static_cast<unsigned int>(bytecode.size()));
}

void TEvaluatorTest::TestDefaultEvaluator() {
    const unsigned int initialRIP = 0x1000;
    TState initialState = MakeInitialState(initialRIP);

    auto defaultEvaluator = MakeDefaultInstructionEvaluator();
    UNIT_ASSERT(defaultEvaluator != nullptr);

    TState testState = initialState;

    llvm::MCInst movRax1 = llvm::MCInstBuilder(llvm::X86::MOV32ri)
        .addReg(llvm::X86::EAX)
        .addImm(1);
    defaultEvaluator->Evaluate(testState, movRax1);

    auto eaxValue = testState.GetImmediateValue(llvm::X86::EAX);
    UNIT_ASSERT(eaxValue.Defined());
    UNIT_ASSERT_EQUAL(*eaxValue, 1);

    auto raxValue = testState.GetImmediateValue(llvm::X86::RAX);
    UNIT_ASSERT(raxValue.Defined());
    UNIT_ASSERT_EQUAL(*raxValue, 1);

    llvm::MCInst leaInst = llvm::MCInstBuilder(llvm::X86::LEA32r)
        .addReg(llvm::X86::EBX)
        .addReg(llvm::X86::RAX)
        .addImm(1)
        .addReg(0)
        .addImm(0x100)
        .addReg(0);

    defaultEvaluator->Evaluate(testState, leaInst);

    auto ebxValue = testState.GetImmediateValue(llvm::X86::EBX);
    UNIT_ASSERT(ebxValue.Defined());
    UNIT_ASSERT_EQUAL(*ebxValue, 0x101);

    auto rbxValue = testState.GetImmediateValue(llvm::X86::RBX);
    UNIT_ASSERT(rbxValue.Defined());
    UNIT_ASSERT_EQUAL(*rbxValue, 0x101);

    llvm::MCInst movRcxRax = llvm::MCInstBuilder(llvm::X86::MOV32rr)
        .addReg(llvm::X86::ECX)
        .addReg(llvm::X86::EAX);

    defaultEvaluator->Evaluate(testState, movRcxRax);

    auto ecxValue = testState.GetImmediateValue(llvm::X86::ECX);
    UNIT_ASSERT(ecxValue.Defined());
    UNIT_ASSERT_EQUAL(*ecxValue, 1);

    auto rcxValue = testState.GetImmediateValue(llvm::X86::RCX);
    UNIT_ASSERT(rcxValue.Defined());
    UNIT_ASSERT_EQUAL(*rcxValue, 1);

    llvm::MCInst lea64_32Inst = llvm::MCInstBuilder(llvm::X86::LEA64_32r)
        .addReg(llvm::X86::EAX)
        .addReg(llvm::X86::RBX)
        .addImm(1)
        .addReg(0)
        .addImm(0x100000000)
        .addReg(0);

    TState testState64_32 = initialState;
    testState64_32.SetImmediate(llvm::X86::RBX, 0x200000000);
    defaultEvaluator->Evaluate(testState64_32, lea64_32Inst);

    auto eaxValue64_32 = testState64_32.GetImmediateValue(llvm::X86::EAX);
    UNIT_ASSERT(eaxValue64_32.Defined());
    UNIT_ASSERT_EQUAL(*eaxValue64_32, 0);

    auto raxValue64_32 = testState64_32.GetImmediateValue(llvm::X86::RAX);
    UNIT_ASSERT(raxValue64_32.Defined());
    UNIT_ASSERT_EQUAL(*raxValue64_32, 0);
}

