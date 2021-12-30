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
std::vector<std::string> lines;

void append(std::string s) {
    lines.push_back(s);
}

void appendTab(std::string s) {
    lines.push_back("\t" + s);
}

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
        appendTab("movq	" + regOrConstant1 + ", " + "%rax");
        appendTab("movl	" + regOrConstant2 + ", " + reg);
        appendTab("cltd");
        appendTab("idivl	" + reg);
        appendTab("movl	%eax, " + reg);
    } else {
        appendTab("movl	" + regOrConstant1 + ", " + reg);
        appendTab(name + "	" + regOrConstant2 + ", " + reg);
    }
}
string CodeGenerate::generate() {
    int i, functionEndNumber = 0;
    FOR (i, 8, 15)
        allRegister[i] = true;
    lines.clear();

    append(".text");
    append(".globl main");
    // append(".p2align	4, 0x90");

    for (auto& function : module->get_functions()) {
        if (function->get_num_basic_blocks()) {
            append("");
            append(function->get_name() + ":");

            appendTab(".cfi_startproc");
            appendTab("pushq	%rbp");
            appendTab("movq	%rsp, %rbp");
            // appendTab("subq	$16, %rsp");

            for (auto basicblock : function->get_basic_blocks()) {
                for (auto instruction : basicblock->get_instructions()) {
                    Value* value = instruction;
                    auto instructionType = instruction->get_instr_type();
                    if (instruction->isBinary())
                        binaryInstGenerate(instruction);

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
