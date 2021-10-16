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

    auto mainFunction = Function::create(FunctionType::get(Int32Type, {}), "main", module);
    auto basicBlock = BasicBlock::create(module, "entry", mainFunction);
    builder->set_insert_point(basicBlock);
    auto retAlloca = builder->create_alloca(Int32Type);
    builder->create_store(CONST_INT(0), retAlloca);

    auto aAlloca = builder->create_alloca(Int32Type);
    auto iAlloca = builder->create_alloca(Int32Type);
    builder->create_store(CONST_INT(10), aAlloca);
    builder->create_store(CONST_INT(0), iAlloca);

    auto checkBasicBlock = BasicBlock::create(module, "checkBasicBlock", mainFunction);
    auto whileBasicBlock = BasicBlock::create(module, "whileBasicBlock", mainFunction);
    auto returnBasicBlock = BasicBlock::create(module, "returnBasicBlock", mainFunction);

    // while 判断 基本块
    builder->create_br(checkBasicBlock);

    builder->set_insert_point(checkBasicBlock);
    auto iLoad = builder->create_load(iAlloca);
    auto icmp = builder->create_icmp_lt(iLoad, CONST_INT(10));
    builder->create_cond_br(icmp, whileBasicBlock, returnBasicBlock);

    // while 内部 基本块
    builder->set_insert_point(whileBasicBlock);
    iLoad = builder->create_load(iAlloca);
    auto addedI = builder->create_iadd(iLoad, CONST_INT(1));
    builder->create_store(addedI, iAlloca);

    iLoad = builder->create_load(iAlloca);
    auto aLoad = builder->create_load(aAlloca);
    auto addedA = builder->create_iadd(aLoad, iLoad);
    builder->create_store(addedA, aAlloca);
    builder->create_br(checkBasicBlock);

    // 返回 基本块
    builder->set_insert_point(returnBasicBlock);
    aLoad = builder->create_load(aAlloca);
    builder->create_ret(aLoad);

    std::cout << module->print();
    delete module;
    return 0;
}
