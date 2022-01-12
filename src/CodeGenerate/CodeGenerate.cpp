#include "CodeGenerate.hpp"
#include "AsmInstruction.hpp"
#include "Position.hpp"

#define floatType module->get_float_type()
#define int32Type module->get_int32_type()
#define int1Type module->get_int1_type()
#define voidType module->get_void_type()

Module* module;

string CodeGenerate::generate() {
    ::module = CodeGenerate::module;
    int i;
    FOR (i, 8, 15)
        allRegister[i] = nullptr;

    string ansCode;
    ansCode += ".text\n";
    ansCode += ".globl main\n";

    int functionEndNumber = 0;
    for (auto& function : module->get_functions()) {
        if (function->get_num_basic_blocks() == 0)
            continue;
        functionEndNumber++;
        auto asmFunction = new AsmFunction(function, functionEndNumber);
        asmFunction->generate();
        ansCode += "\n" + asmFunction->print();
        delete asmFunction;
    }
    return ansCode;
}

void AsmBlock::generate() {
    for (auto instruction : basicBlock->get_instructions()) {
        if (instruction->is_ret())
            retInstGenerate(instruction);
        else if (instruction->isBinary())
            binaryInstGenerate(instruction);
        else if (instruction->is_cmp())
            cmpInstGenerate(instruction);
        else if (instruction->is_zext())
            zextInstGenerate(instruction);
        else if (instruction->is_call())
            callInstGenerate(instruction);
    }
}

void AsmBlock::retInstGenerate(Instruction* instruction) {
    if (instruction->get_num_operand()) {
        auto value = instruction->get_operand(0);
        appendInst(movl, getPosition(value), eax);
    }
    appendInst(addq, ConstInteger(asmFunction->stackSpace), rsp);
    appendInst(popq, rbp);
    appendInst(retq);
}

void AsmBlock::binaryInstGenerate(Instruction* instruction) {
    map<Instruction::OpID, string> opToName;
    opToName[Instruction::add] = addl;
    opToName[Instruction::sub] = subl;
    opToName[Instruction::mul] = imull;

    Value* value = instruction;
    auto instructionType = instruction->get_instr_type();
    auto instName = opToName[instruction->get_instr_type()];
    auto value1 = instruction->get_operand(0);
    auto value2 = instruction->get_operand(1);
    auto regOrConstant1 = getPosition(value1);
    auto regOrConstant2 = getPosition(value2);
    auto reg = getEmptyRegister(value);
    if (instruction->is_div()) {
        appendInst(movq, regOrConstant1, rax);
        appendInst(movl, regOrConstant2, reg);
        appendInst(cltd);
        appendInst(idivl, reg);
        appendInst(movl, eax, reg);
    } else {
        appendInst(movl, regOrConstant1, reg);
        appendInst(instName, regOrConstant2, reg);
    }
}

void AsmBlock::cmpInstGenerate(Instruction* instruction) {
    map<CmpInst::CmpOp, string> opToName;
    opToName[CmpInst::EQ] = sete;
    opToName[CmpInst::NE] = setne;
    opToName[CmpInst::GT] = setg;
    opToName[CmpInst::GE] = setge;
    opToName[CmpInst::LT] = setl;
    opToName[CmpInst::LE] = setle;

    auto cmpInstruction = dynamic_cast<CmpInst*>(instruction);
    auto value1 = instruction->get_operand(0);
    auto value2 = instruction->get_operand(1);
    auto regOrConstant1 = getPosition(value1);
    auto regOrConstant2 = getPosition(value2);
    auto reg = getEmptyRegister(instruction);

    appendInst(movl, regOrConstant1, reg);
    string instName = opToName[cmpInstruction->get_cmp_op()];
    appendInst(cmpl, regOrConstant2, reg);
    appendInst(instName, cl);
    appendInst(movzbl, cl, reg);
}

void AsmBlock::zextInstGenerate(Instruction* instruction) {
    auto rightValue = instruction->get_operand(0);
    valueToRegister[instruction] = valueToRegister[rightValue];
}

void AsmBlock::callInstGenerate(Instruction* instruction) {
    auto operands = instruction->get_operands();
    auto returnType = instruction->get_type();
    auto callFunctionName = operands[0]->get_name();
    int operandNumber = operands.size();
    int i;

    FORDOWN (i, operandNumber - 1, 1)
        appendInst(pushq, getPosition(operands[i]));
    if (returnType == voidType)
        appendInst(call, Position(callFunctionName));
    else if (returnType == int32Type) {
        appendInst(call, Position(callFunctionName));
        appendInst(movl, eax, getAddress(instruction));
        appendInst(movl, eax, getEmptyRegister(instruction));
    }
    appendInst(addq, ConstInteger(8 * (operandNumber - 1)), rsp);
}
