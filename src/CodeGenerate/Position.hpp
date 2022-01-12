#ifndef POSITION_HPP
#define POSITION_HPP

#include "CodeGenerate.hpp"

class Position {
public:
    string name;

    Position() {}
    Position(string name) { this->name = name; }
    Position(const char* name) { this->name = string(name); }
    Position(Value* value) {}
};

class Register : public Position {
public:
    Register() {}
    Register(string name) { this->name = "%" + name; }
    Register(const char* name) { this->name = "%" + string(name); }
};

class MemoryAddress : public Position {
public:
    MemoryAddress(string name) { this->name = "%" + name; }
    MemoryAddress(const char* name) { this->name = "%" + string(name); }
};

class ConstInteger : public Position {
public:
    ConstInteger(int value) { name = "$" + to_string(value); }
};

Register rbp("rbp"), rsp("rsp"), eax("eax"), rax("rax");

map<Value*, Register*> valueToRegister;
map<Value*, MemoryAddress*> valueToAddress;
map<int, Value*> allRegister;

#define Position(x) *new Position(x)
#define Register(x) *new Register(x)
#define MemoryAddress(x) *new MemoryAddress(x)
#define ConstInteger(x) *new ConstInteger(x)

MemoryAddress& getAddress(Value* value) {
    static int top = 0;
    if (valueToAddress.count(value)) {
        top += 4;
        MemoryAddress& ans = MemoryAddress("-" + to_string(top) + "(%rbp)");
        valueToAddress[value] = &ans;
        return ans;
    }
    return *valueToAddress[value];
}

Register& getEmptyRegister(Value* value) {
    int i;
    FOR (i, 8, 15)
        if (allRegister[i] == nullptr) {
            allRegister[i] = value;
            Register& ans = Register("r" + to_string(i) + "d");
            valueToRegister[value] = &ans;
            return ans;
        }
    return Register("NoneRegister");
}

Position& getPosition(Value* value) {
    Register ans();
    auto constantValue = dynamic_cast<ConstantInt*>(value);

    if (constantValue)
        return ConstInteger(constantValue->get_value());
    // return Position("NonePosition");
    return *valueToRegister[value];  // ?
}

#endif
