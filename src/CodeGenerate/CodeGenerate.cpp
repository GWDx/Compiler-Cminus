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
    }
}

void AsmBlock::retInstGenerate(Instruction* instruction) {
    if (instruction->get_num_operand()) {
        auto value = instruction->get_operand(0);
        allInstruction.push_back(AsmInstruction("movl", getPosition(value), eax));
    }
    appendInst("addq", ConstInteger(asmFunction->stackSpace), rsp);
    appendInst("popq", rbp);
    appendInst("retq");
}

void AsmBlock::binaryInstGenerate(Instruction* instruction) {
    map<Instruction::OpID, string> opToName;
    opToName[Instruction::add] = "addl";
    opToName[Instruction::sub] = "subl";
    opToName[Instruction::mul] = "imull";

    Value* value = instruction;
    auto instructionType = instruction->get_instr_type();
    string name = opToName[instruction->get_instr_type()];
    auto value1 = instruction->get_operand(0);
    auto value2 = instruction->get_operand(1);
    auto regOrConstant1 = getPosition(value1);
    auto regOrConstant2 = getPosition(value2);
    auto reg = getEmptyRegister(value);
    if (instruction->is_div()) {
        appendInst("movq", regOrConstant1, rax);
        appendInst("movl", regOrConstant2, reg);
        appendInst("cltd");
        appendInst("idivl", reg);
        appendInst("movl", eax, reg);
    } else {
        appendInst("movl", regOrConstant1, reg);
        appendInst(name, regOrConstant2, reg);
    }
}
