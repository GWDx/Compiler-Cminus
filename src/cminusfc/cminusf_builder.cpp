#include "cminusf_builder.hpp"
#include "logging.hpp"

// use these macros to get constant value
#define CONST_FP(num) ConstantFP::get((float)num, module.get())
#define CONST_INT(num) ConstantInt::get(num, module.get())

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

void CminusfBuilder::visit(ASTProgram& node) {
    for (auto i : node.declarations)
        i->accept(*this);
}

void CminusfBuilder::visit(ASTNum& node) {
    if (node.type == TYPE_INT)
        value = ConstantInt::get(node.i_val, module.get());
    else
        value = ConstantFP::get(node.f_val, module.get());
}

void CminusfBuilder::visit(ASTVarDeclaration& node) {}

void CminusfBuilder::visit(ASTFunDeclaration& node) {
    Type* returnValueType;
    if (node.type == TYPE_VOID)
        returnValueType = Type::get_void_type(module.get());
    else if (node.type == TYPE_INT)
        returnValueType = Type::get_int32_type(module.get());
    auto function = Function::create(FunctionType::get(returnValueType, {}), node.id, module.get());
    auto basicBlock = BasicBlock::create(module.get(), "entry", function);
    builder->set_insert_point(basicBlock);

    node.compound_stmt->accept(*this);

    // if (node.type == TYPE_VOID)
    //     builder->create_void_ret();
    // else
    //     builder->create_ret(CONST_INT(0));
}

void CminusfBuilder::visit(ASTParam& node) {}

void CminusfBuilder::visit(ASTCompoundStmt& node) {
    for (auto i : node.local_declarations)
        i->accept(*this);
    for (auto i : node.statement_list)
        i->accept(*this);
}

void CminusfBuilder::visit(ASTExpressionStmt& node) {}

void CminusfBuilder::visit(ASTSelectionStmt& node) {}

void CminusfBuilder::visit(ASTIterationStmt& node) {}

void CminusfBuilder::visit(ASTVar& node) {}

void CminusfBuilder::visit(ASTReturnStmt& node) {
    if (node.expression == nullptr)
        builder->create_void_ret();
    else {
        node.expression->accept(*this);
        builder->create_ret(value);
    }
}

void CminusfBuilder::visit(ASTAssignExpression& node) {}

void CminusfBuilder::visit(ASTSimpleExpression& node) {
    if (node.additive_expression_r == nullptr)
        node.additive_expression_l->accept(*this);
}

void CminusfBuilder::visit(ASTAdditiveExpression& node) {
    if (node.additive_expression == nullptr)
        node.term->accept(*this);
}

void CminusfBuilder::visit(ASTTerm& node) {
    if (node.term == nullptr)
        node.factor->accept(*this);
}

void CminusfBuilder::visit(ASTCall& node) {}
