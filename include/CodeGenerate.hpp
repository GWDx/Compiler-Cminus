#ifndef CODE_GENERATE_HPP
#define CODE_GENERATE_HPP

#include <map>
#include <string>
#include <vector>
#include "Module.h"

#define FOR(i, l, r) for (i = l; i <= r; i++)
#define FORDOWN(i, r, l) for (i = r; i >= l; i--)

using std::map;
using std::string;
using std::to_string;
using std::vector;

class CodeGenerate {
public:
    Module* module;

    CodeGenerate(Module* m) { module = m; }

    std::string generate();
};

#endif
