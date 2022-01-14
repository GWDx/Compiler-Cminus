#ifndef POSITION_HPP
#define POSITION_HPP

#include <algorithm>
#include "CodeGenerate.hpp"

#define FOR(i, l, r) for (i = l; i <= (int)r; i++)
#define FORDOWN(i, r, l) for (i = r; i >= l; i--)

using std::list;
using std::map;
using std::string;
using std::to_string;
using std::vector;

class Position {
public:
    string name;

    Position() {}
    Position(string name) { this->name = name; }
    Position(Value* value) {}
};

class Register : public Position {
public:
    Register() {}
    Register(string name) { this->name = "%" + name; }
};

class MemoryAddress : public Position {
public:
    MemoryAddress(int offset, Register reg) { this->name = to_string(offset) + "(" + reg.name + ")"; }
    MemoryAddress(string offset, Register reg) { this->name = offset + "(" + reg.name + ")"; }
};

class ConstInteger : public Position {
public:
    ConstInteger(int value) { name = "$" + to_string(value); }
};

Register rbp("rbp"), rsp("rsp"), rip("rip"), eax("eax"), rax("rax"), cl("cl");
Register edi("edi"), esi("esi"), edx("edx"), ecx("ecx"), r8d("r8d"), r9d("r9d");
Register r10d("r10d"), r11d("r11d"), r12d("r12d"), r13d("r13d"), r14d("r14d"), r15d("r15d");
Register xmm0("xmm0"), xmm1("xmm1"), xmm2("xmm2"), xmm3("xmm3"), xmm4("xmm4"), xmm5("xmm5"), xmm6("xmm6"), xmm7("xmm7");
Register xmm8("xmm8"), xmm9("xmm9"), xmm10("xmm10"), xmm11("xmm11"), xmm12("xmm12"), xmm13("xmm13"), xmm14("xmm14"),
    xmm15("xmm15");

vector<Register*> argIntRegister = {&edi, &esi, &edx, &ecx, &r8d, &r9d};
vector<Register*> argFloatRegister = {&xmm0, &xmm1, &xmm2, &xmm3, &xmm4, &xmm5, &xmm6, &xmm7};

map<Register*, Value*> registerToValue;
map<Value*, Register*> valueToRegister;
map<Value*, MemoryAddress*> valueToAddress;
map<string, MemoryAddress*> globalStringToAddress;

#define Position(x) *new Position(x)
#define Register(x) *new Register(x)
#define MemoryAddress(x, y) *new MemoryAddress(x, y)
#define ConstInteger(x) *new ConstInteger(x)

MemoryAddress& getCallAddress(Value* value) {
    static int top = 0;
    if (valueToAddress.count(value) == 0) {
        top += 4;
        MemoryAddress& ans = MemoryAddress(-top, rbp);
        valueToAddress[value] = &ans;
        return ans;
    }
    return *valueToAddress[value];
}

#define floatType module->get_float_type()
#define int32Type module->get_int32_type()
#define int1Type module->get_int1_type()
#define voidType module->get_void_type()

Module* module;

vector<Register*> leastRecentIntRegister = {&r10d, &r11d, &r12d, &r13d, &r14d, &r15d};
vector<Register*> leastRecentFloatRegister = {&xmm8, &xmm9, &xmm10, &xmm11, &xmm12, &xmm13, &xmm14, &xmm15};

void updateRegister(Value* value) {
    auto valueType = value->get_type();
    Register* reg = valueToRegister[value];
    vector<Register*>* leastRecentRegister;
    if (valueType == int32Type or valueType == int1Type)
        leastRecentRegister = &leastRecentIntRegister;
    else if (valueType == floatType)
        leastRecentRegister = &leastRecentFloatRegister;
    auto location = std::find(leastRecentRegister->begin(), leastRecentRegister->end(), reg);
    if (location == leastRecentRegister->end())
        return;
    leastRecentRegister->erase(location);
    leastRecentRegister->push_back(reg);
}

Register& getEmptyRegister(Value* value);

#endif
