#ifndef CODE_GENERATE_HPP
#define CODE_GENERATE_HPP

#include <string>
#include "Module.h"

class CodeGenerate {
public:
    Module* module;

    CodeGenerate(Module* m) { module = m; }

    std::string generate();
};

#endif
