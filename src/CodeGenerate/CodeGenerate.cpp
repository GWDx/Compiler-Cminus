#include "CodeGenerate.hpp"
#include "AsmInstruction.hpp"
#include "Position.hpp"

string CodeGenerate::generate() {
    ::module = CodeGenerate::module;
    int i;
    FOR (i, 8, 15)
        allRegister[i] = nullptr;

    string ansCode;

    int functionEndNumber = 0;
    for (auto& function : module->get_functions()) {
        if (function->get_num_basic_blocks() == 0)
            continue;
        functionEndNumber++;
        auto asmFunction = new AsmFunction(function, functionEndNumber);
        asmFunction->generate();
        ansCode += asmFunction->print();
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
        else if (instruction->is_fp2si())
            fpToSiInstGenerate(instruction);
        else if (instruction->is_si2fp())
            siToFpInstGenerate(instruction);
        else if (instruction->is_call())
            callInstGenerate(instruction);
        else if (instruction->is_br())
            brInstGenerate(instruction);
        else if (instruction->is_phi())
            phiInstGenerate(instruction);
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
    opToName[Instruction::fadd] = addss;
    opToName[Instruction::fsub] = subss;
    opToName[Instruction::fmul] = mulss;
    opToName[Instruction::fdiv] = divss;

    auto instName = opToName[instruction->get_instr_type()];
    auto value1 = instruction->get_operand(0);
    auto value2 = instruction->get_operand(1);
    auto reg = getEmptyRegister(instruction);
    if (instruction->is_div()) {
        appendInst(movq, getPosition(value1), rax);
        appendInst(movl, getPosition(value2), reg);
        appendInst(cltd);
        appendInst(idivl, reg);
        appendInst(movl, eax, reg);
    } else if (instruction->is_add() or instruction->is_sub() or instruction->is_mul()) {
        appendInst(movl, getPosition(value1), reg);
        appendInst(instName, getPosition(value2), reg);
    } else {
        appendInst(movss, getPosition(value1), reg);
        appendInst(instName, getPosition(value2), reg);
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
    auto instName = opToName[cmpInstruction->get_cmp_op()];
    auto value1 = instruction->get_operand(0);
    auto value2 = instruction->get_operand(1);
    auto reg = getEmptyRegister(instruction);

    appendInst(movl, getPosition(value1), reg);
    appendInst(cmpl, getPosition(value2), reg);
    appendInst(instName, cl);
    appendInst(movzbl, cl, reg);
}

void AsmBlock::zextInstGenerate(Instruction* instruction) {
    auto rightValue = instruction->get_operand(0);
    valueToRegister[instruction] = valueToRegister[rightValue];
}

void AsmBlock::fpToSiInstGenerate(Instruction* instruction) {
    auto rightValue = instruction->get_operand(0);
    appendInst(cvttss2si, getPosition(rightValue), getEmptyRegister(instruction));
}

void AsmBlock::siToFpInstGenerate(Instruction* instruction) {
    auto rightValue = instruction->get_operand(0);
    auto reg = getEmptyRegister(instruction);
    appendInst(movl, getPosition(rightValue), eax);
    appendInst(cvtsi2ssl, eax, reg);  // ConstInteger
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

void AsmBlock::brInstGenerate(Instruction* instruction) {
    string functionName = instruction->get_function()->get_name();
    auto operands = instruction->get_operands();
    if (operands.size() == 1) {
        string basicBlockName = operands[0]->get_name();
        string labelName = genLabelName(functionName, basicBlockName);
        appendInst(jmp, Position(labelName));
    } else {
        auto condition = operands[0];
        string basicBlockName1 = operands[1]->get_name();
        string basicBlockName2 = operands[2]->get_name();
        string labelName1 = genLabelName(functionName, basicBlockName1);
        string labelName2 = genLabelName(functionName, basicBlockName2);

        // appendInst(cmpl, valueToRegister[condition]);
        appendInst(cmpl, ConstInteger(0), getPosition(condition));
        appendInst(jne, Position(labelName1));
        appendInst(jmp, Position(labelName2));
    }
}

void AsmBlock::phiInstGenerate(Instruction* instruction) {
    int i;
    vector<AsmInstruction>::iterator iter;
    auto operands = instruction->get_operands();
    auto reg = getEmptyRegister(instruction);
    FOR (i, 0, operands.size() / 2 - 1) {
        auto value = operands[2 * i];
        auto label = operands[2 * i + 1];
        auto basicBlock = static_cast<BasicBlock*>(label);
        auto asmBlock = basicBlockToAsmBlock[basicBlock];

        auto& allInstruction = asmBlock->allInstruction;
        for (iter = allInstruction.end() - 1; iter >= allInstruction.begin(); iter--)
            if (iter->name != jmp and iter->name != jne)  //
                break;
        allInstruction.insert(iter + 1, AsmInstruction(movl, getPosition(value), reg));
    }
}
