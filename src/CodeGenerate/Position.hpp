#ifndef POSITION_HPP
#define POSITION_HPP

#include "CodeGenerate.hpp"

#define FOR(i, l, r) for (i = l; i <= (int)r; i++)
#define FORDOWN(i, r, l) for (i = r; i >= l; i--)

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

Register rbp("rbp"), rsp("rsp"), rip("rip"), eax("eax"), rax("rax"), cl("cl"), xmm0("xmm0");

map<Value*, Position*> valueToPosition;
map<Value*, MemoryAddress*> valueToAddress;
map<string, bool> allRegister;

#define Position(x) *new Position(x)
#define Register(x) *new Register(x)
#define MemoryAddress(x, y) *new MemoryAddress(x, y)
#define ConstInteger(x) *new ConstInteger(x)

MemoryAddress& getAddress(Value* value) {
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

Register& getIntRegister() {
    int i;
    FOR (i, 8, 15) {
        string reg = "r" + to_string(i) + "d";
        if (allRegister[reg] == false) {
            allRegister[reg] = true;
            return Register(reg);
        }
    }
    return Register("NoneRegister");
}

Register& getFloatRegister() {
    int i;
    FOR (i, 8, 15) {
        string reg = "xmm" + to_string(i);
        if (allRegister[reg] == false) {
            allRegister[reg] = true;
            return Register(reg);
        }
    }
    return Register("NoneRegister");
}

Register& getEmptyRegister(Value* value) {
    int i;
    auto valueType = value->get_type();
    if (valueType == int32Type or valueType == int1Type) {
        Register& reg = getIntRegister();
        valueToPosition[value] = &reg;
        return reg;
    } else if (valueType == floatType) {
        Register& reg = getFloatRegister();
        valueToPosition[value] = &reg;
        return reg;
    }
    return Register("NoneRegister");
}

#endif
