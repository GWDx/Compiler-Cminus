#include "CodeGenerate.hpp"
#include <map>
#include <string>
#include <vector>

using std::map;
using std::string;
using std::to_string;
using std::vector;
Module* module;

#define FOR(i, l, r) for (i = l; i <= r; i++)
#define FORDOWN(i, r, l) for (i = r; i >= l; i--)

#define floatType module->get_float_type()
#define int32Type module->get_int32_type()
#define int1Type module->get_int1_type()
#define voidType module->get_void_type()

map<int, Value*> allRegister;
map<Value*, string> valueToRegister, valueToAddress;
vector<string> lines;

void append(std::string s = "") {
    lines.push_back(s);
}

void appendTab(std::string s) {
    lines.push_back("\t" + s);
}

string getEmptyRegister(Value* value) {
    int i;
    FOR (i, 8, 15)
        if (allRegister[i] == nullptr) {
            allRegister[i] = value;
            string ans = "%r" + to_string(i) + "d";
            valueToRegister[value] = ans;
            return ans;
        }
    return "";
}

string getAddress(Value* value) {
    static int top = 0;
    if (valueToAddress[value] == "") {
        top += 4;
        string ans = "-" + to_string(top) + "(%rbp)";
        valueToAddress[value] = ans;
        return ans;
    }
    return valueToAddress[value];
}

string valueToRegOrConstant(Value* value) {
    string ans;
    auto constantValue = dynamic_cast<ConstantInt*>(value);
    if (constantValue)
        ans = "$" + to_string(constantValue->get_value());
    else
        ans = valueToRegister[value];
    return ans;
}

void binaryInstGenerate(Instruction* instruction) {
    map<Instruction::OpID, string> opToName;
    opToName[Instruction::add] = "addl";
    opToName[Instruction::sub] = "subl";
    opToName[Instruction::mul] = "imull";
    // opToName[Instruction::sdiv] = "idivl";

    Value* value = instruction;
    auto instructionType = instruction->get_instr_type();
    string name = opToName[instruction->get_instr_type()];

    auto value1 = instruction->get_operand(0);
    auto value2 = instruction->get_operand(1);
    auto regOrConstant1 = valueToRegOrConstant(value1);
    auto regOrConstant2 = valueToRegOrConstant(value2);
    auto reg = getEmptyRegister(value);
    if (instruction->is_div()) {
        appendTab("movq	" + regOrConstant1 + ", %rax");
        appendTab("movl	" + regOrConstant2 + ", " + reg);
        appendTab("cltd");
        appendTab("idivl	" + reg);
        appendTab("movl	%eax, " + reg);
    } else {
        appendTab("movl	" + regOrConstant1 + ", " + reg);
        appendTab(name + "	" + regOrConstant2 + ", " + reg);
    }
}

void callInstGenerate(Instruction* instruction) {
    // auto callInstruction = dynamic_cast<CallInst*>(instruction);
    auto operands = instruction->get_operands();
    auto returnType = instruction->get_type();
    auto functionName = operands[0]->get_name();
    int operandNumber = operands.size();
    int i;

    FORDOWN (i, operandNumber - 1, 1)
        appendTab("pushq	" + valueToRegOrConstant(operands[i]));
    if (returnType == voidType)
        appendTab("call	" + functionName);
    else if (returnType == int32Type) {
        appendTab("call	" + functionName);
        appendTab("movl	%eax, " + getAddress(instruction));
        appendTab("movl	%eax, " + getEmptyRegister(instruction));
    }
    FORDOWN (i, operandNumber - 1, 1)
        appendTab("popq	%rax");
}

string CodeGenerate::generate() {
    ::module = CodeGenerate::module;
    int i, functionEndNumber = 0;
    FOR (i, 8, 15)
        allRegister[i] = nullptr;
    lines.clear();

    append(".text");
    append(".globl main");
    // append(".p2align	4, 0x90");

    for (auto& function : module->get_functions()) {
        if (function->get_num_basic_blocks()) {
            append();
            append(function->get_name() + ":");

            appendTab(".cfi_startproc");
            appendTab("pushq	%rbp");
            appendTab("movq	%rsp, %rbp");
            appendTab("subq	$16, %rsp");  // conditional

            int position = 8;
            for (auto arg : function->get_args()) {
                position += 8;
                appendTab("movl	" + to_string(position) + "(%rbp), " + getEmptyRegister(arg));
            }

            for (auto basicblock : function->get_basic_blocks()) {
                for (auto instruction : basicblock->get_instructions()) {
                    Value* value = instruction;
                    auto instructionType = instruction->get_instr_type();
                    if (instruction->isBinary())
                        binaryInstGenerate(instruction);

                    if (instruction->is_call())
                        callInstGenerate(instruction);

                    if (instructionType == Instruction::ret) {
                        // auto returnInstruction = dynamic_cast<ReturnInst*>(instruction);
                        if (instruction->get_num_operand()) {
                            auto value = instruction->get_operand(0);
                            auto regOrConstant = valueToRegOrConstant(value);
                            appendTab("movl	" + regOrConstant + ", %eax");
                        }
                        appendTab("addq	$16, %rsp");  // conditional
                        appendTab("popq	%rbp");
                        appendTab("retq");
                    }
                }
            }
            append(".Lfunc_end" + to_string(functionEndNumber) + ":");
            appendTab(".cfi_endproc");
            functionEndNumber++;
        }
    }

    string ansCode;
    for (auto line : lines)
        ansCode = ansCode + line + "\n";
    return ansCode;
}
