#include "CodeGenerate.hpp"
#include <map>
#include <string>
#include <vector>

using std::map;
using std::string;
using std::to_string;

#define FOR(i, l, r) for (i = l; i <= r; i++)

map<int, bool> allRegister;
map<Value*, string> valueToRegister;

string getEmptyRegister(Value* value) {
    int i;
    FOR (i, 8, 15)
        if (allRegister[i]) {
            allRegister[i] = false;
            string ans = "%r" + to_string(i) + "d";
            valueToRegister[value] = ans;
            return ans;
        }
    return "";
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

string CodeGenerate::generate() {
    int i, functionEndNumber = 0;
    FOR (i, 8, 15)
        allRegister[i] = true;

    append(".text");
    append(".globl main");
    append(".p2align	4, 0x90");

    for (auto& function : module->get_functions()) {
        if (function->get_num_basic_blocks()) {
            append("");
            append(function->get_name() + ":");

            appendTab(".cfi_startproc");
            appendTab("pushq	%rbp");
            appendTab("movq	%rsp, %rbp");
            // appendTab("subp	%rsp, 16");

            for (auto basicblock : function->get_basic_blocks()) {
                for (auto instruction : basicblock->get_instructions()) {
                    Value* value = instruction;
                    auto instructionType = instruction->get_instr_type();
                    if (instructionType == Instruction::add) {
                        auto value1 = instruction->get_operand(0);
                        auto value2 = instruction->get_operand(1);
                        auto regOrConstant1 = valueToRegOrConstant(value1);
                        auto regOrConstant2 = valueToRegOrConstant(value2);

                        auto reg = getEmptyRegister(value);
                        appendTab("movl	" + regOrConstant1 + ", " + reg);
                        appendTab("addl	" + regOrConstant2 + ", " + reg);
                    }

                    if (instructionType == Instruction::ret) {
                        // auto returnInstruction = dynamic_cast<ReturnInst*>(instruction);
                        if (instruction->get_num_operand()) {
                            auto value = instruction->get_operand(0);
                            auto regOrConstant = valueToRegOrConstant(value);
                            appendTab("movl	" + regOrConstant + ", %eax");
                        }
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
