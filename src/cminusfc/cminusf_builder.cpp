#include "cminusf_builder.hpp"
#include "logging.hpp"

// use these macros to get constant value
#define CONST_FP(num) ConstantFP::get((float)num, module.get())
#define CONST_INT(num) ConstantInt::get(num, module.get())

#define accept accept(*this)

// You can define global variables here
// to store state

/*
 * use CMinusfBuilder::Scope to construct scopes
 * scope.enter: enter a new scope
 * scope.exit: exit current scope
 * scope.push: add a new binding to current scope
 * scope.find: find and return the value bound to the name
 */

Value* value;
Function* function;

void CminusfBuilder::visit(ASTProgram& node) {
    for (auto i : node.declarations)
        i->accept;
}

void CminusfBuilder::visit(ASTNum& node) {
    if (node.type == TYPE_INT)
        value = CONST_INT(node.i_val);
    else
        value = CONST_FP(node.f_val);
}

void CminusfBuilder::visit(ASTVarDeclaration& node) {}

void CminusfBuilder::visit(ASTFunDeclaration& node) {
    Type* returnValueType;
    if (node.type == TYPE_INT)
        returnValueType = Type::get_int32_type(module.get());
    else if (node.type == TYPE_FLOAT)
        returnValueType = Type::get_float_type(module.get());
    else
        returnValueType = Type::get_void_type(module.get());
    function = Function::create(FunctionType::get(returnValueType, {}), node.id, module.get());
    auto basicBlock = BasicBlock::create(module.get(), "entry", function);
    builder->set_insert_point(basicBlock);

    node.compound_stmt->accept;

    // if (node.type == TYPE_VOID)
    //     builder->create_void_ret();
    // else
    //     builder->create_ret(CONST_INT(0));
}

void CminusfBuilder::visit(ASTParam& node) {}

void CminusfBuilder::visit(ASTCompoundStmt& node) {
    scope.enter();
    for (auto i : node.local_declarations)
        i->accept;
    for (auto i : node.statement_list)
        i->accept;
    scope.exit();
}

void CminusfBuilder::visit(ASTExpressionStmt& node) {
    if (node.expression != nullptr)
        node.expression->accept;
}

void CminusfBuilder::visit(ASTSelectionStmt& node) {
    node.expression->accept;

    auto valueType = value->get_type()->get_type_id();
    assert(valueType == Type::IntegerTyID or valueType == Type::FloatTyID);
    Instruction* cmp;
    if (valueType == Type::IntegerTyID)
        cmp = builder->create_icmp_ne(value, CONST_INT(0));
    else
        cmp = builder->create_fcmp_ne(value, CONST_FP(0));

    auto trueBasicBlock = BasicBlock::create(module.get(), "trueBasicBlock", function);
    auto followingBasicblock = BasicBlock::create(module.get(), "followingBasicblock", function);
    if (node.else_statement == nullptr) {
        builder->create_cond_br(cmp, trueBasicBlock, followingBasicblock);

        builder->set_insert_point(trueBasicBlock);
        node.if_statement->accept;
        builder->create_br(followingBasicblock);
    } else {
        auto falseBasicBlock = BasicBlock::create(module.get(), "falseBasicBlock", function);
        builder->create_cond_br(cmp, trueBasicBlock, falseBasicBlock);

        builder->set_insert_point(trueBasicBlock);
        node.if_statement->accept;
        builder->create_br(followingBasicblock);

        builder->set_insert_point(falseBasicBlock);
        node.else_statement->accept;
        builder->create_br(followingBasicblock);
    }
    builder->set_insert_point(followingBasicblock);
}

void CminusfBuilder::visit(ASTIterationStmt& node) {}

void CminusfBuilder::visit(ASTVar& node) {}

void CminusfBuilder::visit(ASTReturnStmt& node) {
    if (node.expression == nullptr)
        builder->create_void_ret();
    else {
        node.expression->accept;
        builder->create_ret(value);
    }
}

void CminusfBuilder::visit(ASTAssignExpression& node) {}

void CminusfBuilder::visit(ASTSimpleExpression& node) {
    if (node.additive_expression_r == nullptr)
        node.additive_expression_l->accept;
}

void CminusfBuilder::visit(ASTAdditiveExpression& node) {
    if (node.additive_expression == nullptr)
        node.term->accept;
}

void CminusfBuilder::visit(ASTTerm& node) {
    if (node.term == nullptr)
        node.factor->accept;
}

void CminusfBuilder::visit(ASTCall& node) {
    std::vector<Value*> values;
    for (auto i : node.args) {
        i->accept;
        values.push_back(value);
    }

    auto callFunction = scope.find(node.id);
    auto call = builder->create_call(callFunction, values);
}
