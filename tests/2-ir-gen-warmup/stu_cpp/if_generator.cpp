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
    Type* FloatPointType = Type::get_float_type(module);

    auto mainFunction = Function::create(FunctionType::get(Int32Type, {}), "main", module);
    auto basicBlock = BasicBlock::create(module, "entry", mainFunction);
    builder->set_insert_point(basicBlock);
    auto retAlloca = builder->create_alloca(Int32Type);
    builder->create_store(CONST_INT(0), retAlloca);

    auto aAlloca = builder->create_alloca(FloatPointType);
    builder->create_store(CONST_FP(5.555), aAlloca);
    auto aLoad = builder->create_load(aAlloca);
    auto fcmp = builder->create_fcmp_gt(aLoad, CONST_FP(1));

    auto trueBasicBlock = BasicBlock::create(module, "trueBasicBlock", mainFunction);
    auto falseBasicBlock = BasicBlock::create(module, "falseBasicBlock", mainFunction);

    builder->create_cond_br(fcmp, trueBasicBlock, falseBasicBlock);
    builder->set_insert_point(trueBasicBlock);
    builder->create_ret(CONST_INT(233));

    builder->set_insert_point(falseBasicBlock);
    builder->create_ret(CONST_INT(0));

    std::cout << module->print();
    delete module;
    return 0;
}
