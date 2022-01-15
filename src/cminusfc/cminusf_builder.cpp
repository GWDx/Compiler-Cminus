// 彭炫超、甘文迪

#include "cminusf_builder.hpp"

// use these macros to get constant value
#define CONST_FP(num) ConstantFP::get((float)num, module.get())
#define CONST_INT(num) ConstantInt::get(num, module.get())

#define floatType module->get_float_type()
#define int32Type module->get_int32_type()
#define int1Type module->get_int1_type()

// You can define global variables here
// to store state

/*
 * use CMinusfBuilder::Scope to construct scopes
 * scope.enter: enter a new scope
 * scope.exit: exit current scope
 * scope.push: add a new binding to current scope
 * scope.find: find and return the value bound to the name
 */

Value* expressionValue;
Value* varAddress;
Value* paramAlloca;
Function* function;
Type* returnValueType;

Type* cminusTypeToType(std::unique_ptr<Module>& module, CminusType t) {
    switch (t) {
        case CminusType::TYPE_FLOAT:
            return floatType;
        case CminusType::TYPE_INT:
            return int32Type;
        case CminusType::TYPE_VOID:
            return module->get_void_type();
        default:
            return nullptr;
    }
}

Value* compareWith0(std::unique_ptr<Module>& module, IRBuilder* builder, Value* inputValue) {
    auto valueType = inputValue->get_type();
    Value* cmp;
    if (valueType == int1Type)
        cmp = inputValue;
    else if (valueType == int32Type)
        cmp = builder->create_icmp_ne(inputValue, CONST_INT(0));
    else
        cmp = builder->create_fcmp_ne(inputValue, CONST_FP(0));
    return cmp;
}

Value* typeConvert(std::unique_ptr<Module>& module, IRBuilder* builder, Value* inputValue, Type* targetType) {
    Value* ansValue = inputValue;
    auto valueType = inputValue->get_type();

    if (valueType == int32Type and targetType == floatType)
        ansValue = builder->create_sitofp(inputValue, floatType);
    else if (valueType == floatType and targetType == int32Type)
        ansValue = builder->create_fptosi(inputValue, int32Type);
    else if (valueType == int1Type and targetType == int32Type)
        ansValue = builder->create_zext(inputValue, int32Type);
    else if (valueType == int1Type and targetType == floatType) {
        auto tempValue = builder->create_zext(inputValue, int32Type);
        ansValue = builder->create_sitofp(tempValue, floatType);
    }
    assert(ansValue->get_type() == targetType);
    return ansValue;
}

void CminusfBuilder::visit(ASTProgram& node) {
    for (auto i : node.declarations)
        i->accept(*this);
}

void CminusfBuilder::visit(ASTNum& node) {
    if (node.type == TYPE_INT)
        expressionValue = CONST_INT(node.i_val);
    else
        expressionValue = CONST_FP(node.f_val);
}

void CminusfBuilder::visit(ASTVarDeclaration& node) {
    Type* T;
    T = cminusTypeToType(module, node.type);
    Value* varAlloca;
    if (node.num)  // array type
        T = module->get_array_type(T, node.num->i_val);
    if (scope.in_global()) {
        Constant* init = ConstantZero::get(T, module.get());
        varAlloca = GlobalVariable::create(node.id, module.get(), T, 0, init);
    } else
        varAlloca = builder->create_alloca(T);
    scope.push(node.id, varAlloca);
}

void CminusfBuilder::visit(ASTFunDeclaration& node) {
    std::vector<Type*> vars;
    for (auto param : node.params) {
        if (param->isarray) {
            if (param->type == TYPE_INT)
                vars.push_back(module->get_int32_ptr_type());
            else
                vars.push_back(module->get_float_ptr_type());
        } else {
            if (param->type == TYPE_INT)
                vars.push_back(int32Type);
            else
                vars.push_back(floatType);
        }
    }
    returnValueType = cminusTypeToType(module, node.type);
    function = Function::create(FunctionType::get(returnValueType, vars), node.id, module.get());

    scope.push(node.id, function);
    scope.enter();

    auto enterbb = BasicBlock::create(module.get(), "enterBB", function);
    builder->set_insert_point(enterbb);

    //处理参数
    auto arg = function->arg_begin();
    for (auto param : node.params) {
        param->accept(*this);
        builder->create_store(*arg, paramAlloca);
        arg++;
    }

    node.compound_stmt->accept(*this);
    scope.exit();

    // 可能会产生 2 句 ret
    if (node.type == TYPE_INT)
        builder->create_ret(CONST_INT(0));
    else if (node.type == TYPE_FLOAT)
        builder->create_ret(CONST_FP(0));
    else
        builder->create_void_ret();
}

void CminusfBuilder::visit(ASTParam& node) {
    // CminusType type;
    // std::string id;
    // true if it is array param
    // bool isarray;
    Type* T;
    T = cminusTypeToType(module, node.type);
    if (node.isarray) {
        paramAlloca = builder->create_alloca(Type::get_pointer_type(T));
    } else {
        paramAlloca = builder->create_alloca(T);
    }
    scope.push(node.id, paramAlloca);
}

void CminusfBuilder::visit(ASTCompoundStmt& node) {
    scope.enter();
    for (auto i : node.local_declarations)
        i->accept(*this);
    for (auto i : node.statement_list)
        i->accept(*this);
    scope.exit();
}

void CminusfBuilder::visit(ASTExpressionStmt& node) {
    if (node.expression != nullptr)
        node.expression->accept(*this);
}

void CminusfBuilder::visit(ASTSelectionStmt& node) {
    node.expression->accept(*this);
    auto cmp = compareWith0(module, builder, expressionValue);

    auto trueBasicBlock = BasicBlock::create(module.get(), "", function);
    auto followingBasicblock = BasicBlock::create(module.get(), "", function);
    if (node.else_statement == nullptr) {
        builder->create_cond_br(cmp, trueBasicBlock, followingBasicblock);

        builder->set_insert_point(trueBasicBlock);
        node.if_statement->accept(*this);
        builder->create_br(followingBasicblock);
    } else {
        auto falseBasicBlock = BasicBlock::create(module.get(), "", function);
        builder->create_cond_br(cmp, trueBasicBlock, falseBasicBlock);

        builder->set_insert_point(trueBasicBlock);
        node.if_statement->accept(*this);
        builder->create_br(followingBasicblock);

        builder->set_insert_point(falseBasicBlock);
        node.else_statement->accept(*this);
        builder->create_br(followingBasicblock);
    }
    builder->set_insert_point(followingBasicblock);
}

void CminusfBuilder::visit(ASTIterationStmt& node) {
    // if （expression） goto body else goto end
    // body:stmt, end
    // end
    auto whileBB = BasicBlock::create(module.get(), "", function);
    builder->create_br(whileBB);
    builder->set_insert_point(whileBB);
    node.expression->accept(*this);
    auto cmp = compareWith0(module, builder, expressionValue);

    auto bodyBB = BasicBlock::create(module.get(), "", function);
    builder->set_insert_point(bodyBB);
    node.statement->accept(*this);
    builder->create_br(whileBB);  // ??
    auto endBB = BasicBlock::create(module.get(), "", function);
    builder->set_insert_point(whileBB);
    builder->create_cond_br(cmp, bodyBB, endBB);
    builder->set_insert_point(endBB);
}

void CminusfBuilder::visit(ASTVar& node) {  // var -> ID | ID[expression]
    if (node.expression == nullptr)
        varAddress = scope.find(node.id);
    else {
        auto varAlloca = scope.find(node.id);
        node.expression->accept(*this);

        expressionValue = typeConvert(module, builder, expressionValue, int32Type);
        auto cmp = builder->create_icmp_lt(expressionValue, CONST_INT(0));

        auto trueBasicBlock = BasicBlock::create(module.get(), "", function);
        auto followingBasicblock = BasicBlock::create(module.get(), "", function);
        builder->create_cond_br(cmp, trueBasicBlock, followingBasicblock);

        builder->set_insert_point(trueBasicBlock);
        auto neg_idx_except_fun = scope.find("neg_idx_except");
        builder->create_call(neg_idx_except_fun, {});
        builder->create_br(followingBasicblock);

        builder->set_insert_point(followingBasicblock);
        assert(varAlloca->get_type()->get_type_id() == Type::PointerTyID);
        // auto elementType = ((PointerType*)varAlloca)->get_element_type();
        auto elementType = varAlloca->get_type()->get_pointer_element_type();
        if (elementType->get_type_id() == Type::ArrayTyID)
            varAddress = builder->create_gep(varAlloca, {CONST_INT(0), expressionValue});
        else {
            Value* var = builder->create_load(varAlloca);
            varAddress = builder->create_gep(var, {expressionValue});
        }
    }
    expressionValue = builder->create_load(varAddress);  // 对 var 赋值时会产生冗余
    if (expressionValue->get_type()->get_type_id() == Type::ArrayTyID) {
        auto varAlloca = scope.find(node.id);
        auto var0Address = builder->create_gep(varAlloca, {CONST_INT(0), CONST_INT(0)});
        expressionValue = var0Address;
    }
}

void CminusfBuilder::visit(ASTReturnStmt& node) {
    if (node.expression == nullptr)
        builder->create_void_ret();
    else {
        node.expression->accept(*this);
        expressionValue = typeConvert(module, builder, expressionValue, returnValueType);
        builder->create_ret(expressionValue);
    }
}

void CminusfBuilder::visit(ASTAssignExpression& node) {
    if (node.var == nullptr)
        node.expression->accept(*this);
    else {
        node.var->accept(*this);
        auto recordVarAddress = varAddress;

        node.expression->accept(*this);
        auto recordVarType = recordVarAddress->get_type()->get_pointer_element_type();
        expressionValue = typeConvert(module, builder, expressionValue, recordVarType);
        builder->create_store(expressionValue, recordVarAddress);
    }
}

void CminusfBuilder::visit(ASTSimpleExpression& node) {
    node.additive_expression_l->accept(*this);
    if (node.additive_expression_r != nullptr) {
        Value* lhs = expressionValue;
        auto lhsType = lhs->get_type();

        node.additive_expression_r->accept(*this);
        Value* rhs = expressionValue;
        auto rhsType = rhs->get_type();

        if (lhsType == int32Type and rhsType == int32Type)
            switch (node.op) {
                case OP_LE:
                    expressionValue = builder->create_icmp_le(lhs, rhs);
                    break;
                case OP_LT:
                    expressionValue = builder->create_icmp_lt(lhs, rhs);
                    break;
                case OP_GT:
                    expressionValue = builder->create_icmp_gt(lhs, rhs);
                    break;
                case OP_GE:
                    expressionValue = builder->create_icmp_ge(lhs, rhs);
                    break;
                case OP_EQ:
                    expressionValue = builder->create_icmp_eq(lhs, rhs);
                    break;
                case OP_NEQ:
                    expressionValue = builder->create_icmp_ne(lhs, rhs);
                    break;
                default:
                    break;
            }
        else {
            lhs = typeConvert(module, builder, lhs, floatType);
            rhs = typeConvert(module, builder, rhs, floatType);

            switch (node.op) {
                case OP_LE:
                    expressionValue = builder->create_fcmp_le(lhs, rhs);
                    break;
                case OP_LT:
                    expressionValue = builder->create_fcmp_lt(lhs, rhs);
                    break;
                case OP_GT:
                    expressionValue = builder->create_fcmp_gt(lhs, rhs);
                    break;
                case OP_GE:
                    expressionValue = builder->create_fcmp_ge(lhs, rhs);
                    break;
                case OP_EQ:
                    expressionValue = builder->create_fcmp_eq(lhs, rhs);
                    break;
                case OP_NEQ:
                    expressionValue = builder->create_fcmp_ne(lhs, rhs);
                    break;
                default:
                    break;
            }
        }
    }
}

void CminusfBuilder::visit(ASTAdditiveExpression& node) {
    if (node.additive_expression == nullptr)
        node.term->accept(*this);
    else {  // additive_expression addop term
        node.additive_expression->accept(*this);
        auto leftValue = expressionValue;
        auto leftValueType = leftValue->get_type();

        node.term->accept(*this);
        auto rightValue = expressionValue;
        auto rightValueType = rightValue->get_type();

        if (leftValueType == int32Type and rightValueType == int32Type) {
            if (node.op == OP_PLUS)
                expressionValue = builder->create_iadd(leftValue, rightValue);
            else
                expressionValue = builder->create_isub(leftValue, rightValue);
        } else {
            leftValue = typeConvert(module, builder, leftValue, floatType);
            rightValue = typeConvert(module, builder, rightValue, floatType);

            if (node.op == OP_PLUS)
                expressionValue = builder->create_fadd(leftValue, rightValue);
            else
                expressionValue = builder->create_fsub(leftValue, rightValue);
        }
    }
}

void CminusfBuilder::visit(ASTTerm& node) {
    if (node.term == nullptr)
        node.factor->accept(*this);
    else {
        node.term->accept(*this);
        auto leftValue = expressionValue;
        auto leftValueType = leftValue->get_type();

        node.factor->accept(*this);
        auto rightValue = expressionValue;
        auto rightValueType = rightValue->get_type();

        if (leftValueType == int32Type and rightValueType == int32Type) {
            if (node.op == OP_MUL)
                expressionValue = builder->create_imul(leftValue, rightValue);
            else
                expressionValue = builder->create_isdiv(leftValue, rightValue);
        } else {
            leftValue = typeConvert(module, builder, leftValue, floatType);
            rightValue = typeConvert(module, builder, rightValue, floatType);

            if (node.op == OP_MUL)
                expressionValue = builder->create_fmul(leftValue, rightValue);
            else
                expressionValue = builder->create_fdiv(leftValue, rightValue);
        }
    }
}

void CminusfBuilder::visit(ASTCall& node) {
    std::vector<Value*> values;
    auto callFunction = scope.find(node.id);
    auto callFunctionType = ((Function*)callFunction)->get_function_type();
    // auto callFunctionTypeArgsNumber = callFunctionType->get_num_of_args();

    int size = node.args.size();
    for (int i = 0; i < size; i++) {
        node.args[i]->accept(*this);
        auto callFunctionTypeArgsI = callFunctionType->get_param_type(i);
        expressionValue = typeConvert(module, builder, expressionValue, callFunctionTypeArgsI);
        values.push_back(expressionValue);
    }

    expressionValue = builder->create_call(callFunction, values);
}
