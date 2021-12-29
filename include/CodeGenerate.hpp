#ifndef CODE_GENERATE_HPP
#define CODE_GENERATE_HPP

#include <string>
#include "Module.h"

class CodeGenerate {
public:
    Module* module;
    std::vector<std::string> lines;

    CodeGenerate(Module* m) {
        module = m;
        lines.clear();
    }
    void append(std::string s) { lines.push_back(s); };
    void appendTab(std::string s) { lines.push_back("\t" + s); };

    std::string generate();
};

#endif
