#include "CodeGenerate.hpp"
#include <string>
#include <vector>

std::string CodeGenerate::generate() {
    int functionEndNumber = 0;
    append(".text");
    append(".globl main");
    append(".p2align	4, 0x90");

    for (auto& function : module->get_functions()) {
        if (function->get_num_basic_blocks()) {
            append("");
            append(function->get_name() + ":");

            appendTab(".cfi_startproc");
            appendTab("retq");

            for (auto basicblock : function->get_basic_blocks()) {
            }
            append(".Lfunc_end" + std::to_string(functionEndNumber) + ":");
            appendTab(".cfi_endproc");
            functionEndNumber++;
        }
    }

    std::string ansCode;
    for (auto line : lines)
        ansCode = ansCode + line + "\n";
    return ansCode;
}
