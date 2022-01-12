#include "CodeGenerate.hpp"
#include "AsmInstruction.hpp"
#include "Position.hpp"

#define floatType module->get_float_type()
#define int32Type module->get_int32_type()
#define int1Type module->get_int1_type()
#define voidType module->get_void_type()

Module* module;

void AsmBlock::generate() {
    for (auto instruction : basicBlock->get_instructions()) {
        auto instructionType = instruction->get_instr_type();
        if (instructionType == Instruction::ret) {
            if (instruction->get_num_operand()) {
                auto value = instruction->get_operand(0);
                allInstruction.push_back(AsmInstruction("movl", getPosition(value), eax));
            }
            allInstruction.push_back(AsmInstruction("addq", ConstInteger(asmFunction->stackSpace), rsp));
            allInstruction.push_back(AsmInstruction("popq", rbp));
            allInstruction.push_back(AsmInstruction("retq"));
        }
    }
}

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
