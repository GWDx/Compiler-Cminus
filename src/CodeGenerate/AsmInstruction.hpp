#ifndef Asm_INSTRUCTION_HPP
#define Asm_INSTRUCTION_HPP

#include "CodeGenerate.hpp"
#include "Position.hpp"

using std::string;
using std::vector;

class AsmInstruction {
public:
    string name;
    vector<Position> positions;

    string print() {
        int i;
        int size = positions.size();
        string ans = name + "\t";
        if (name.length() < 4)
            ans += "\t";
        if (size > 0) {
            FOR (i, 0, size - 2)
                ans += positions[i].name + ", ";
            ans += positions[size - 1].name;
        }
        return ans;
    }

    AsmInstruction(string name) { this->name = name; };

    AsmInstruction(string name, Position& p1) {
        this->name = name;
        positions.push_back(p1);
    }

    AsmInstruction(string name, Position& p1, Position& p2) {
        this->name = name;
        positions.push_back(p1);
        positions.push_back(p2);
    }
};

class AsmFunction;

class AsmBlock {
public:
    string name;
    vector<AsmInstruction> allInstruction;
    BasicBlock* basicBlock;
    AsmFunction* asmFunction;

    AsmBlock(AsmFunction* asmFunction, BasicBlock* basicBlock) {
        this->asmFunction = asmFunction;
        this->basicBlock = basicBlock;
        name = basicBlock->get_name();
    }

    void generate();
};

class AsmFunction {
public:
    vector<AsmBlock> allBlock;
    vector<AsmInstruction> initInst;
    int stackSpace = 16;

    Function* function;
    int functionEndNumber;

    AsmFunction(Function* function, int functionEndNumber) {
        this->function = function;
        this->functionEndNumber = functionEndNumber;

        for (auto basicBlock : function->get_basic_blocks())
            allBlock.push_back(AsmBlock(this, basicBlock));
    }

    void generate() {
        initInst.push_back(AsmInstruction("pushq", rbp));
        initInst.push_back(AsmInstruction("movq", rsp, rbp));
        initInst.push_back(AsmInstruction("subq", ConstInteger(stackSpace), rsp));

        int position = 8;
        for (auto arg : function->get_args()) {
            position += 8;
            initInst.push_back(AsmInstruction("movl", Position(to_string(position) + "(%rbp)"), getPosition(arg)));
        }
        for (auto& block : allBlock)
            block.generate();
    }

#define append(s) ans += s + "\n"
#define appendTab(s) ans += string("\t") + s + "\n"

    string print() {
        string ans;
        string functionName = function->get_name();

        append(function->get_name() + ":");
        appendTab(".cfi_startproc");
        for (auto instruction : initInst)
            appendTab(instruction.print());

        for (auto block : allBlock) {
            append("." + functionName + "_" + block.basicBlock->get_name() + ":");
            for (auto instruction : block.allInstruction)
                appendTab(instruction.print());
        }

        append(".Lfunc_end" + to_string(functionEndNumber) + ":");
        appendTab(".cfi_endproc");
        return ans;
    }
};

#endif
