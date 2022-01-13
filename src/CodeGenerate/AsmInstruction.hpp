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
string addss("addss"), subss("subss"), mulss("mulss"), divss("divss");
string cvttss2si("cvttss2si"), cvtsi2ssl("cvtsi2ssl");
string sete("sete"), setne("setne"), setg("setg"), setge("setge"), setl("setl"), setle("setle");
string seta("seta"), setae("setae"), setb("setb"), setbe("setbe");
string cmpl("cmpl"), ucomiss("ucomiss"), jmp("jmp"), jne("jne");
string movq("movq"), movl("movl"), movzbl("movzbl"), movss("movss"), cltd("cltd");
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

    Position& getPosition(Value* value);
    void generate();

    void retInstGenerate(Instruction* instruction);
    void binaryInstGenerate(Instruction* instruction);
    void cmpInstGenerate(Instruction* instruction);
    void fcmpInstGenerate(Instruction* instruction);
    void zextInstGenerate(Instruction* instruction);
    void fpToSiInstGenerate(Instruction* instruction);
    void siToFpInstGenerate(Instruction* instruction);
    void callInstGenerate(Instruction* instruction);
    void brInstGenerate(Instruction* instruction);
    void phiInstGenerate(Instruction* instruction);
    void loadInstGenerate(Instruction* instruction);
    void storeInstGenerate(Instruction* instruction);
};

string genLabelName(string functionName, string basicBlockName) {
    return "." + functionName + "_" + basicBlockName;
}

map<BasicBlock*, AsmBlock*> basicBlockToAsmBlock;

class AsmFunction {
public:
    vector<AsmBlock> allBlock;
    vector<AsmInstruction> initInst;
    int stackSpace = 16;
    vector<int> allConstInteger;
    vector<string> allConstLabel;
    Function* function;
    int functionEndNumber;
    string functionName;

    AsmFunction(Function* function, int functionEndNumber) {
        this->function = function;
        this->functionEndNumber = functionEndNumber;
        functionName = function->get_name();
        int i;
        for (auto basicBlock : function->get_basic_blocks())
            allBlock.push_back(AsmBlock(this, basicBlock));
        FOR (i, 0, allBlock.size() - 1)
            basicBlockToAsmBlock[allBlock[i].basicBlock] = &allBlock[i];
    }

    void generate() {
        initInst.push_back(AsmInstruction(pushq, rbp));
        initInst.push_back(AsmInstruction(movq, rsp, rbp));
        initInst.push_back(AsmInstruction(subq, ConstInteger(stackSpace), rsp));  // ?

        int i = 0;
        Position* position;
        for (auto arg : function->get_args()) {
            if (i < 6)
                position = functionArgRegister[i];
            else
                position = &MemoryAddress(8 * (i - 6), rbp);
            if (arg->get_type() == int32Type)
                initInst.push_back(AsmInstruction(movl, *position, getEmptyRegister(arg)));
            else
                initInst.push_back(AsmInstruction(movss, *position, getEmptyRegister(arg)));
        }
        for (auto& block : allBlock)
            block.generate();
    }

    string appendConst(int value) {
        string labelName = genLabelName(functionName, to_string(allConstInteger.size()));
        allConstInteger.push_back(value);
        allConstLabel.push_back(labelName);
        return labelName;
    }

#define appendLine(s) ans += s + string("\n")
#define appendLineTab(s) ans += string("\t") + s + "\n"

    string print() {
        int i;
        string ans;
        FOR (i, 0, allConstLabel.size() - 1) {
            appendLine(allConstLabel[i] + ":");
            appendLineTab(".long	" + to_string(allConstInteger[i]));
        }
        appendLine(".text");
        appendLine(".globl " + functionName);
        appendLine(functionName + ":");
        appendLineTab(".cfi_startproc");
        for (auto instruction : initInst)
            appendLineTab(instruction.print());

        for (auto block : allBlock) {
            appendLine(genLabelName(functionName, block.name) + ":");
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

Position& AsmBlock::getPosition(Value* value) {
    Register ans();
    auto constantInt = dynamic_cast<ConstantInt*>(value);
    auto constantFloat = dynamic_cast<ConstantFP*>(value);
    auto globalVar = dynamic_cast<GlobalVariable*>(value);

    if (constantInt)
        return ConstInteger(constantInt->get_value());
    if (constantFloat) {
        float constantFloatValue = constantFloat->get_value();
        string label = asmFunction->appendConst(*(int*)&constantFloatValue);
        return MemoryAddress(label, rip);
    }
    if (globalVar)
        return MemoryAddress(globalVar->get_name(), rip);
    if (valueToPosition[value])
        return *valueToPosition[value];
    return Position("NonePosition");
}

#endif
