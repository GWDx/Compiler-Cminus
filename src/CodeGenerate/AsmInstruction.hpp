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

string addq("addq"), addl("addl"), subl("subl"), subq("subq"), imull("imull"), idivl("idivl");
string movq("movq"), movl("movl"), cltd("cltd");
string popq("popq"), pushq("pushq"), retq("retq"), call("call");

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
    void appendInst(string name) { allInstruction.push_back(AsmInstruction(name)); }
    void appendInst(string name, Position& p1) { allInstruction.push_back(AsmInstruction(name, p1)); }
    void appendInst(string name, Position& p1, Position& p2) { allInstruction.push_back(AsmInstruction(name, p1, p2)); }

    void generate();
    void retInstGenerate(Instruction* instruction);
    void binaryInstGenerate(Instruction* instruction);
    void callInstGenerate(Instruction* instruction);
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
        initInst.push_back(AsmInstruction(pushq, rbp));
        initInst.push_back(AsmInstruction(movq, rsp, rbp));
        initInst.push_back(AsmInstruction(subq, ConstInteger(stackSpace), rsp));

        int position = 8;
        for (auto arg : function->get_args()) {
            position += 8;
            initInst.push_back(AsmInstruction(movl, MemoryAddress(position, rbp), getEmptyRegister(arg)));
        }
        for (auto& block : allBlock)
            block.generate();
    }

#define appendLine(s) ans += s + "\n"
#define appendLineTab(s) ans += string("\t") + s + "\n"

    string print() {
        string ans;
        string functionName = function->get_name();

        appendLine(function->get_name() + ":");
        appendLineTab(".cfi_startproc");
        for (auto instruction : initInst)
            appendLineTab(instruction.print());

        for (auto block : allBlock) {
            appendLine("." + functionName + "_" + block.basicBlock->get_name() + ":");
            for (auto instruction : block.allInstruction)
                appendLineTab(instruction.print());
        }
        appendLine(".Lfunc_end" + to_string(functionEndNumber) + ":");
        appendLineTab(".cfi_endproc");
        return ans;
    }
#undef appendLine
#undef appendLineTab
};

#endif
