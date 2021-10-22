#include "BasicBlock.h"
#include "Constant.h"
#include "Function.h"
#include "IRBuilder.h"
#include "Module.h"
#include "Type.h"

#include <iostream>

#define DEBUG_OUTPUT std::clog << __LINE__ << std::endl;  // 输出行号的简单示例

#define CONST_INT(num) ConstantInt::get(num, module)
#define CONST_FP(num) ConstantFP::get(num, module)

int main() {
    auto module = new Module("Cminus code");
    auto builder = new IRBuilder(nullptr, module);
    Type* Int32Type = Type::get_int32_type(module);

    // main 函数
    auto mainFunction = Function::create(FunctionType::get(Int32Type, {}), "main", module);
    auto basicBlock = BasicBlock::create(module, "entry", mainFunction);
    builder->set_insert_point(basicBlock);
    auto retAlloca = builder->create_alloca(Int32Type);
    builder->create_store(CONST_INT(0), retAlloca);

    auto* arrayType = ArrayType::get(Int32Type, 10);
    auto aAlloca = builder->create_alloca(arrayType);

    auto a0Gep = builder->create_gep(aAlloca, {CONST_INT(0), CONST_INT(0)});
    builder->create_store(CONST_INT(10), a0Gep);

    a0Gep = builder->create_gep(aAlloca, {CONST_INT(0), CONST_INT(0)});
    auto a1Gep = builder->create_gep(aAlloca, {CONST_INT(0), CONST_INT(1)});
    auto a0Load = builder->create_load(a0Gep);
    auto multiplied = builder->create_imul(a0Load, CONST_INT(2));
    builder->create_store(multiplied, a1Gep);

    a1Gep = builder->create_gep(aAlloca, {CONST_INT(0), CONST_INT(1)});
    auto a1Load = builder->create_load(a1Gep);

    builder->create_ret(a1Load);

    std::cout << module->print();
    delete module;
    return 0;
}
