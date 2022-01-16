#include "CodeGenerate.hpp"
#include "AsmInstruction.hpp"
#include "Position.hpp"

string CodeGenerate::generate() {
    ::module = CodeGenerate::module;
    int i;
    FOR (i, 8, 15)
        reg32To64["%r" + to_string(i) + "d"] = "%r" + to_string(i);
    reg32To64["%edi"] = "%rdi";
    reg32To64["%esi"] = "%rsi";
    reg32To64["%edx"] = "%rdx";
    reg32To64["%ecx"] = "%rcx";

    string ansCode;
    for (auto globalVar : module->get_global_variable()) {
        string name = globalVar->get_name();
        int size = 4;
        if (globalVar->get_type()->get_pointer_element_type()->get_type_id() == Type::ArrayTyID) {
            auto pointType = globalVar->get_type()->get_pointer_element_type();
            auto arrayType = static_cast<ArrayType*>(pointType);
            size = 4 * arrayType->get_num_of_elements();
        }
        ansCode += ".comm	" + name + "," + to_string(size) + ",4\n";
        globalName[name] = true;
    }
    if (ansCode.size() > 0)
        ansCode += "\n";

    int functionEndNumber = 0;
    for (auto& function : module->get_functions()) {
        if (function->get_num_basic_blocks() == 0)
            continue;
        functionEndNumber++;
        asmFunction = new AsmFunction(function, functionEndNumber);
        asmFunction->generate();
        ansCode += asmFunction->print() + "\n";
        delete asmFunction;
    }
    // for (auto& i : allAllocPosition)
    //     delete i;
    return ansCode;
}

void AsmBlock::normalGenerate() {
    instructionInsertLocation = &normalInstructions;
    for (auto instruction : basicBlock->get_instructions()) {
        if (instruction->isBinary())
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
        else if (instruction->is_alloca())
            allocaInstGenerate(instruction);
        else if (instruction->is_gep())
            gepInstGenerate(instruction);
        else if (instruction->is_ret() and instruction->get_num_operand())
            getPosition(instruction->get_operand(0));
    }
    stash();
}

void AsmBlock::endGenerate() {
    for (auto instruction : basicBlock->get_instructions())
        if (instruction->is_ret())
            retInstGenerate(instruction);
}

void AsmBlock::retInstGenerate(Instruction* instruction) {
    instructionInsertLocation = &endInstructions;
    if (instruction->get_num_operand()) {
        auto value = instruction->get_operand(0);
        if (value->get_type() == int32Type)
            appendInst(movl, getPosition(value), eax);
        else
            appendInst(movss, getPosition(value), xmm0);
    }
    appendInst(addq, ConstInteger(stackSpace), rsp);
    appendInst(popq, rbp);
    appendInst(retq);
    instructionInsertLocation = &normalInstructions;
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

    appendInst(movl, getPosition(value1), eax);
    appendInst(cmpl, getPosition(value2), eax);
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
    auto tempReg = getEmptyRegister(tempFloat);
    auto reg = getEmptyRegister(instruction);

    appendInst(movss, getPosition(value1), tempReg);
    appendInst(ucomiss, getPosition(value2), tempReg);
    appendInst(instName, cl);  // ?
    appendInst(movzbl, cl, reg);
}

void AsmBlock::zextInstGenerate(Instruction* instruction) {
    auto rightValue = instruction->get_operand(0);
    auto reg = getEmptyRegister(instruction);
    appendInst(movl, getPosition(rightValue), reg);
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
    instructionInsertLocation = &stackInstruction;
    FOR (i, 1, operandNumber - 1) {
        Position& position = getPosition(operands[i]);
        auto type = operands[i]->get_type();
        if (type == int32Type) {
            if (intRegisterIndex < argIntRegister.size())
                appendInst(movl, position, *argIntRegister[intRegisterIndex++]);
            else
                appendInst(pushq, position);
        } else if (type == floatType) {
            if (floatRegisterIndex < argFloatRegister.size())
                appendInst(movss, position, *argFloatRegister[floatRegisterIndex++]);
            else
                appendInst(pushq, position);  // ?
        } else {
            if (intRegisterIndex < argIntRegister.size())
                appendInst(movq, position, *argIntRegister[intRegisterIndex++]);
            else
                appendInst(pushq, position);
        }
    }
    for (auto iter = stackInstruction.rbegin(); iter != stackInstruction.rend(); iter++)
        normalInstructions.push_back(*iter);

    instructionInsertLocation = &normalInstructions;
    stash();
    appendInst(call, Position(callFunctionName));
    if (returnType == int32Type)
        appendInst(movl, eax, getEmptyRegister(instruction));
    else if (returnType == floatType)
        appendInst(movss, xmm0, getEmptyRegister(instruction));
    if (operandNumber >= 7)
        appendInst(addq, ConstInteger(8 * (operandNumber - 7)), rsp);
}

void AsmBlock::brInstGenerate(Instruction* instruction) {
    instructionInsertLocation = &endInstructions;
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

        appendInst(cmpl, ConstInteger(0), getPosition(condition));
        appendInst(jne, Position(labelName1));
        appendInst(jmp, Position(labelName2));
    }
    instructionInsertLocation = &normalInstructions;
}

void AsmBlock::phiInstGenerate(Instruction* instruction) {
    int i;
    auto operands = instruction->get_operands();
    FOR (i, 0, operands.size() / 2 - 1) {
        auto value = operands[2 * i];
        auto label = operands[2 * i + 1];
        auto basicBlock = static_cast<BasicBlock*>(label);
        auto asmBlock = basicBlockToAsmBlock[basicBlock];
        auto& endInstructions = asmBlock->endInstructions;
        endInstructions.insert(endInstructions.begin(), AsmInstruction(movl, eax, getAddress(instruction)));
        endInstructions.insert(endInstructions.begin(), AsmInstruction(movl, getPosition(value), eax));
    }
}

void AsmBlock::loadInstGenerate(Instruction* instruction) {
    auto rightValue = instruction->get_operand(0);
    auto type = rightValue->get_type();
    auto name = rightValue->get_name();
    auto reg = getEmptyRegister(instruction);
    appendInst(movq, getPosition(rightValue), rax);
    appendInst(movq, MemoryAddress(0, rax), reg);
}

void AsmBlock::storeInstGenerate(Instruction* instruction) {
    auto value = instruction->get_operand(0);
    auto targetValue = instruction->get_operand(1);
    auto type = targetValue->get_type()->get_pointer_element_type();
    auto name = targetValue->get_name();
    string moveName;
    Register* reg;
    if (type == int32Type) {
        moveName = movl;
        reg = &getEmptyRegister(tempInt);
    } else {
        moveName = movss;
        reg = &getEmptyRegister(tempFloat);
    }
    appendInst(movq, getPosition(targetValue), rax);
    appendInst(moveName, getPosition(value), *reg);
    appendInst(moveName, *reg, MemoryAddress(0, rax));
}

void AsmBlock::allocaInstGenerate(Instruction* instruction) {
    auto allocaInst = dynamic_cast<AllocaInst*>(instruction);
    auto allocaType = allocaInst->get_alloca_type();
    if (allocaType->get_type_id() == Type::ArrayTyID) {
        auto arrayType = static_cast<ArrayType*>(allocaType);
        int nums = arrayType->get_num_of_elements();
        stackSpace += nums * 4;
    }
    getAddress(instruction);
}

void AsmBlock::gepInstGenerate(Instruction* instruction) {
    auto operands = instruction->get_operands();
    auto pointer = operands[0];
    auto& reg = getEmptyRegister(instruction);
    Value* index;
    if (operands.size() == 3)
        index = operands[2];
    else
        index = operands[1];
    appendInst(movq, getPosition(pointer), reg);
    appendInst(movl, getPosition(index), eax);
    appendInst(imull, ConstInteger(4), eax);
    appendInst(addq, rax, reg);
}
