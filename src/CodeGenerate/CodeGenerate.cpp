#include "CodeGenerate.hpp"
#include <string>
#include <vector>

using std::string;
using std::to_string;

string CodeGenerate::generate() {
    int functionEndNumber = 0;
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

            for (auto basicblock : function->get_basic_blocks()) {
                for (auto instruction : basicblock->get_instructions())
                    if (instruction->get_instr_type() == Instruction::ret) {
                        // auto returnInstruction = dynamic_cast<ReturnInst*>(instruction);
                        if (instruction->get_num_operand()) {
                            auto value = instruction->get_operand(0);
                            auto constantValue = dynamic_cast<ConstantInt*>(value);
                            appendTab("mov	$" + to_string(constantValue->get_value()) + ", %eax");
                        }
                        appendTab("popq	%rbp");
                        appendTab("retq");
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
