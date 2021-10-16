#include "BasicBlock.h"
#include "Constant.h"
#include "Function.h"
#include "IRBuilder.h"
#include "Module.h"
#include "Type.h"

#include <iostream>

#define DEBUG_OUTPUT std::cerr << __LINE__ << std::endl;  // 输出行号的简单示例

#define CONST_INT(num) ConstantInt::get(num, module)
#define CONST_FP(num) ConstantFP::get(num, module)  // 得到常数值的表示,方便后面多次用到

int main() {
    auto module = new Module("Cminus code");  // module name是什么无关紧要
    auto builder = new IRBuilder(nullptr, module);
    Type* Int32Type = Type::get_int32_type(module);

    std::vector<Type*> Ints(1, Int32Type);
    auto calleeFunctionType = FunctionType::get(Int32Type, Ints);
    auto calleeFunction = Function::create(calleeFunctionType, "callee", module);
    auto basicBlock = BasicBlock::create(module, "entry", calleeFunction);
    builder->set_insert_point(basicBlock);
    auto aAlloca = builder->create_alloca(Int32Type);

    std::vector<Value*> args;
    for (auto arg = calleeFunction->arg_begin(); arg != calleeFunction->arg_end(); arg++)
        args.push_back(*arg);
    builder->create_store(args[0], aAlloca);

    auto aLoad = builder->create_load(aAlloca);
    auto multiplied = builder->create_imul(CONST_INT(2), aLoad);
    builder->create_ret(multiplied);

    // main
    auto mainFunction = Function::create(FunctionType::get(Int32Type, {}), "main", module);
    basicBlock = BasicBlock::create(module, "entry", mainFunction);
    builder->set_insert_point(basicBlock);
    auto retAlloca = builder->create_alloca(Int32Type);
    builder->create_store(CONST_INT(0), retAlloca);

    auto call = builder->create_call(calleeFunction, {CONST_INT(110)});
    builder->create_ret(call);

    std::cout << module->print();
    delete module;
    return 0;
}
