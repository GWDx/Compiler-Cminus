#include "CodeGenerate.hpp"
#include "AsmInstruction.hpp"
#include "Position.hpp"

string CodeGenerate::generate() {
    ::module = CodeGenerate::module;
    int i;
    string ansCode;
    for (auto globalVar : module->get_global_variable())
        ansCode += ".comm	" + globalVar->get_name() + ",4,4\n";
    if (ansCode.size() > 0)
        ansCode += "\n";

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
        else if (instruction->is_fcmp())
            fcmpInstGenerate(instruction);
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
        else if (instruction->is_load())
            loadInstGenerate(instruction);
        else if (instruction->is_store())
            storeInstGenerate(instruction);
    }
}

void AsmBlock::retInstGenerate(Instruction* instruction) {
    if (instruction->get_num_operand()) {
        auto value = instruction->get_operand(0);
        if (value->get_type() == int32Type)
            appendEndInst(movl, getPosition(value), eax);
        else
            appendEndInst(movss, getPosition(value), xmm0);
    }
    appendEndInst(addq, ConstInteger(asmFunction->stackSpace), rsp);
    appendEndInst(popq, rbp);
    appendEndInst(retq);
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
    auto temp = getIntRegister();
    auto reg = getEmptyRegister(instruction);

    appendInst(movl, getPosition(value1), temp);
    appendInst(cmpl, getPosition(value2), temp);
    appendInst(instName, cl);
    appendInst(movzbl, cl, reg);
}

void AsmBlock::fcmpInstGenerate(Instruction* instruction) {
    map<FCmpInst::CmpOp, string> opToName;
    opToName[FCmpInst::EQ] = sete;  // ? setnp
    opToName[FCmpInst::NE] = setne;
    opToName[FCmpInst::GT] = seta;
    opToName[FCmpInst::GE] = setae;
    opToName[FCmpInst::LT] = setb;  // ?
    opToName[FCmpInst::LE] = setbe;

    auto cmpInstruction = dynamic_cast<FCmpInst*>(instruction);
    auto instName = opToName[cmpInstruction->get_cmp_op()];
    auto value1 = instruction->get_operand(0);
    auto value2 = instruction->get_operand(1);
    auto temp = getFloatRegister();
    auto reg = getEmptyRegister(instruction);

    appendInst(movss, getPosition(value1), temp);
    appendInst(ucomiss, getPosition(value2), temp);
    appendInst(instName, cl);  // ?
    appendInst(movzbl, cl, reg);
}

void AsmBlock::zextInstGenerate(Instruction* instruction) {
    auto rightValue = instruction->get_operand(0);
    valueToPosition[instruction] = valueToPosition[rightValue];
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
    int i, intRegisterIndex = 0, floatRegisterIndex = 0;
    vector<AsmInstruction> stackInstruction;
    FOR (i, 1, operandNumber - 1) {
        Position& position = getPosition(operands[i]);
        if (operands[i]->get_type() == int32Type) {
            if (intRegisterIndex < argIntRegister.size())
                stackInstruction.push_back(AsmInstruction(movl, position, *argIntRegister[intRegisterIndex++]));
            else
                stackInstruction.push_back(AsmInstruction(pushq, position));
        } else {
            if (floatRegisterIndex < argFloatRegister.size())
                stackInstruction.push_back(AsmInstruction(movss, position, *argFloatRegister[floatRegisterIndex++]));
            else
                stackInstruction.push_back(AsmInstruction(pushq, position));  // ?
        }
    }
    for (auto iter = stackInstruction.rbegin(); iter != stackInstruction.rend(); iter++)
        normalInstructions.push_back(*iter);

    appendInst(call, Position(callFunctionName));
    if (returnType == int32Type) {
        appendInst(movl, eax, getAddress(instruction));
        appendInst(movl, eax, getEmptyRegister(instruction));
    } else if (returnType == floatType) {
        appendInst(movss, xmm0, getAddress(instruction));
        appendInst(movss, xmm0, getEmptyRegister(instruction));
    }
    if (operandNumber >= 7)
        appendInst(addq, ConstInteger(8 * (operandNumber - 7)), rsp);
}

void AsmBlock::brInstGenerate(Instruction* instruction) {
    string functionName = instruction->get_function()->get_name();
    auto operands = instruction->get_operands();
    if (operands.size() == 1) {
        string basicBlockName = operands[0]->get_name();
        string labelName = genLabelName(functionName, basicBlockName);
        appendEndInst(jmp, Position(labelName));
    } else {
        auto condition = operands[0];
        string basicBlockName1 = operands[1]->get_name();
        string basicBlockName2 = operands[2]->get_name();
        string labelName1 = genLabelName(functionName, basicBlockName1);
        string labelName2 = genLabelName(functionName, basicBlockName2);

        appendEndInst(cmpl, ConstInteger(0), getPosition(condition));
        appendEndInst(jne, Position(labelName1));
        appendEndInst(jmp, Position(labelName2));
    }
}

void AsmBlock::phiInstGenerate(Instruction* instruction) {
    int i;
    auto operands = instruction->get_operands();
    auto reg = getEmptyRegister(instruction);
    FOR (i, 0, operands.size() / 2 - 1) {
        auto value = operands[2 * i];
        auto label = operands[2 * i + 1];
        auto basicBlock = static_cast<BasicBlock*>(label);
        auto asmBlock = basicBlockToAsmBlock[basicBlock];
        auto& endInstructions = asmBlock->endInstructions;
        endInstructions.insert(endInstructions.begin(), AsmInstruction(movl, getPosition(value), reg));
    }
}

void AsmBlock::loadInstGenerate(Instruction* instruction) {
    auto rightValue = instruction->get_operand(0);
    valueToPosition[instruction] = &getPosition(rightValue);
}

void AsmBlock::storeInstGenerate(Instruction* instruction) {
    auto value = instruction->get_operand(0);
    auto rightValue = instruction->get_operand(1);
    if (value->get_type() == int32Type)
        appendInst(movl, getPosition(value), getPosition(rightValue));
    else
        appendInst(movss, getPosition(value), getPosition(rightValue));
}
