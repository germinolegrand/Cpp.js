#include "Interpreter.h"

#include <iostream>

Interpreter::Interpreter()
{
    //ctor
}

void Interpreter::feed(Parser::ParseTree tree)
{
    m_parseTrees.push_back(std::move(tree));
    ExecutionContext ctx {
        Realm{},
        var{},
        var{{
            {"console", {{
                {"log", var(
                    [](auto args){
                        std::copy(std::begin(args), std::end(args), std::ostream_iterator<var>(std::cout));
                        std::cout << '\n';
                        return var{};
                    })
                }
            }}},
            {"exit", var(
                [](auto args)->var{
                    exit(args.size() >= 1 ? static_cast<int>(args[0].to_double()) : 0);
                })
            }
        }},
        m_parseTrees.back().root(),
        m_parseTrees.back().root(),
        m_parseTrees.back().root(),
        {}
    };
    m_executionStack.push(std::move(ctx));
}

var Interpreter::execute()
{
    auto& ctx = m_executionStack.top();
    ctx.currentNode = ctx.code;
    ctx.previousNode = ctx.currentNode;
    auto endNode = ctx.code.end();
    while(ctx.currentNode != endNode){
        execute_step();
    }
    try{
        return ctx.calculated.at(ctx.code);
    }catch(...){
        return {};
    }
}

auto Interpreter::execute_step() -> CompletionRecord
{
    auto& ctx = m_executionStack.top();
    auto saveCurrentNode = ctx.currentNode;
    CompletionRecord cr = execute_Node(saveCurrentNode);
    if(cr.type == CompletionRecord::Type::Normal){
        if(!cr.value.is_undefined()){
            ctx.calculated.try_emplace(saveCurrentNode, cr.value);
        }
        ctx.previousNode = saveCurrentNode;
        if(ctx.currentNode == saveCurrentNode){
            ctx.currentNode = saveCurrentNode.parent();
        }
    } else if(cr.type == CompletionRecord::Type::Return) {
        m_executionStack.pop();
        auto& parentCtx = m_executionStack.top();
        parentCtx.calculated.insert_or_assign(parentCtx.currentNode, cr.value);
        parentCtx.previousNode = parentCtx.currentNode;
        parentCtx.currentNode = parentCtx.currentNode.parent();
    } else if(cr.type == CompletionRecord::Type::Break) {
        throw unimplemented_error("CompletionRecord::Type::Break");
    } else if(cr.type == CompletionRecord::Type::Continue) {
        throw unimplemented_error("CompletionRecord::Type::Continue");
    } else if(cr.type == CompletionRecord::Type::Throw) {
        throw unimplemented_error("CompletionRecord::Type::Throw");
    }
    return cr;
}

auto Interpreter::execute_Node(Parser::ParseNode node) -> CompletionRecord
{
    auto& nodeVal = *node;
    if(std::holds_alternative<Parser::Statement>(nodeVal)){
        return execute_Statement(node);
    }
    if(std::holds_alternative<Parser::Operation>(nodeVal)){
        return execute_Operation(node);
    }
    if(std::holds_alternative<Parser::VarUse>(nodeVal)){
        return execute_VarUse(node);
    }
    if(std::holds_alternative<Parser::VarDecl>(nodeVal)){
        return execute_VarDecl(node);
    }
    return {CompletionRecord::Type::Normal, std::get<Parser::Literal>(*node), {}};
}

auto Interpreter::execute_Statement(Parser::ParseNode node) -> CompletionRecord
{
    using Statement = Parser::Statement;
    switch(std::get<Statement>(*node)){
    case Statement::STM_TranslationUnit:
        return execute_STM_TranslationUnit(node);
    case Statement::STM_Expression:
        return execute_STM_Expression(node);
    case Statement::STM_Block:
        return execute_STM_Block(node);
    default:
        throw unimplemented_error(std::get<Statement>(*node));
    }
}

auto Interpreter::execute_Operation(Parser::ParseNode node) -> CompletionRecord
{
    using Operation = Parser::Operation;
    switch(std::get<Operation>(*node)){
    case Operation::OPR_Grouping:
        return execute_OPR_Grouping(node);
    case Operation::OPR_JsonObject:
        return execute_OPR_JsonObject(node);
//    case Operation::OPR_ArrayObject:
//        return execute_OPR_ArrayObject(node);
//    case Operation::OPR_Function:
//        return execute_OPR_Function(node);
    case Operation::OPR_MemberAccess:
        return execute_OPR_MemberAccess(node);
//    case Operation::OPR_New:
//        return execute_OPR_New(node);
    case Operation::OPR_Call:
        return execute_OPR_Call(node);
//    case Operation::OPR_PostfixIncrement:
//        return execute_OPR_PostfixIncrement(node);
//    case Operation::OPR_PostfixDecrement:
//        return execute_OPR_PostfixDecrement(node);
//    case Operation::OPR_PrefixIncrement:
//        return execute_OPR_PrefixIncrement(node);
//    case Operation::OPR_PrefixDecrement:
//        return execute_OPR_PrefixDecrement(node);
//    case Operation::OPR_LogicalNot:
//        return execute_OPR_LogicalNot(node);
//    case Operation::OPR_BitwiseNot:
//        return execute_OPR_BitwiseNot(node);
//    case Operation::OPR_UnaryPlus:
//        return execute_OPR_UnaryPlus(node);
//    case Operation::OPR_UnaryNegation:
//        return execute_OPR_UnaryNegation(node);
//    case Operation::OPR_Typeof:
//        return execute_OPR_Typeof(node);
//    case Operation::OPR_Void:
//        return execute_OPR_Void(node);
//    case Operation::OPR_Delete:
//        return execute_OPR_Delete(node);
    case Operation::OPR_Multiplication:
        return execute_OPR_BinaryOperation<operator* >(node);
//    case Operation::OPR_Exponentiation:
//        return execute_OPR_Exponentiation(node);
    case Operation::OPR_Division:
        return execute_OPR_BinaryOperation<operator/ >(node);
    case Operation::OPR_Remainder:
        return execute_OPR_BinaryOperation<operator% >(node);
    case Operation::OPR_Addition:
        return execute_OPR_BinaryOperation<operator+ >(node);
    case Operation::OPR_Subtraction:
        return execute_OPR_BinaryOperation<operator- >(node);
//    case Operation::OPR_BitwiseLeftShift:
//        return execute_OPR_BinaryOperation<operator<< >(node);
//    case Operation::OPR_BitwiseRightShift:
//        return execute_OPR_BinaryOperation<operator>> >(node);
//    case Operation::OPR_BitwiseUnsignedRightShift:
//        return execute_OPR_BinaryOperation<operator>>> >(node);
//    case Operation::OPR_LessThan:
//        return execute_OPR_LessThan(node);
//    case Operation::OPR_LessThanOrEqual:
//        return execute_OPR_LessThanOrEqual(node);
//    case Operation::OPR_GreaterThan:
//        return execute_OPR_GreaterThan(node);
//    case Operation::OPR_GreaterThanOrEqual:
//        return execute_OPR_GreaterThanOrEqual(node);
//    case Operation::OPR_In:
//        return execute_OPR_In(node);
//    case Operation::OPR_InstanceOf:
//        return execute_OPR_InstanceOf(node);
//    case Operation::OPR_Equality:
//        return execute_OPR_Equality(node);
//    case Operation::OPR_Inequality:
//        return execute_OPR_Inequality(node);
//    case Operation::OPR_StrictEquality:
//        return execute_OPR_StrictEquality(node);
//    case Operation::OPR_StrictInequality:
//        return execute_OPR_StrictInequality(node);
//    case Operation::OPR_BitwiseAND:
//        return execute_OPR_BitwiseAND(node);
//    case Operation::OPR_BitwiseXOR:
//        return execute_OPR_BitwiseXOR(node);
//    case Operation::OPR_BitwiseOR:
//        return execute_OPR_BitwiseOR(node);
//    case Operation::OPR_LogicalAND:
//        return execute_OPR_LogicalAND(node);
//    case Operation::OPR_LogicalOR:
//        return execute_OPR_LogicalOR(node);
//    case Operation::OPR_Conditional:
//        return execute_OPR_Conditional(node);
    case Operation::OPR_Assignment:
        return execute_OPR_AssignmentOperation(node);
    case Operation::OPR_AdditionAssignment:
        return execute_OPR_AssignmentOperation<&var::operator+= >(node);
    case Operation::OPR_SubtractAssignment:
        return execute_OPR_AssignmentOperation<&var::operator-= >(node);
    case Operation::OPR_MultiplicationAssignment:
        return execute_OPR_AssignmentOperation<&var::operator*= >(node);
    case Operation::OPR_DivisionAssignment:
        return execute_OPR_AssignmentOperation<&var::operator/= >(node);
    case Operation::OPR_RemainderAssignment:
        return execute_OPR_AssignmentOperation<&var::operator%= >(node);
//    case Operation::OPR_LeftShiftAssignment:
//        return execute_OPR_AssignmentOperation<var::operator<<= >(node);
//    case Operation::OPR_RightShiftAssignment:
//        return execute_OPR_AssignmentOperation<var::operator>>= >(node);
//    case Operation::OPR_UnsignedRightShiftAssignment:
//        return execute_OPR_AssignmentOperation<var::operator>>>= >(node);
//    case Operation::OPR_BitwiseANDAssignment:
//        return execute_OPR_AssignmentOperation<var::operator&= >(node);
//    case Operation::OPR_BitwiseXORAssignment:
//        return execute_OPR_AssignmentOperation<var::operator^= >(node);
//    case Operation::OPR_BitwiseORAssignment:
//        return execute_OPR_AssignmentOperation<var::operator|= >(node);
//    case Operation::OPR_Yield:
//        return execute_OPR_Yield(node);
//    case Operation::OPR_Spread:
//        return execute_OPR_Spread(node);
//    case Operation::OPR_Comma:
//        return execute_OPR_Comma(node);
    default:
        throw unimplemented_error(std::get<Operation>(*node));
    }
}

auto Interpreter::execute_VarUse(Parser::ParseNode node) -> CompletionRecord
{
    auto& name = std::get<Parser::VarUse>(*node).name;
    auto& variable = context().environment[name];
    return {CompletionRecord::Type::Normal, variable, {}};
}

auto Interpreter::execute_VarDecl(Parser::ParseNode node) -> CompletionRecord
{
    if(context().previousNode == node.parent()){
        if(node.empty()){
            auto& name = std::get<Parser::VarDecl>(*node).name;
            auto& variable = context().environment[name];
            return {CompletionRecord::Type::Normal, variable, {}};
        }

        context().currentNode = node.begin();
        return {CompletionRecord::Type::Normal, {}, {}};
    }

    auto& name = std::get<Parser::VarDecl>(*node).name;
    auto& variable = context().environment[name];
    variable = movePreviousCalculated(node);
    return {CompletionRecord::Type::Normal, variable, {}};
}

auto Interpreter::execute_STM_TranslationUnit(Parser::ParseNode node) -> CompletionRecord
{
    if(context().previousNode == node){
        context().currentNode = node.begin();
        return CompletionRecord::Normal();
    }
    context().currentNode = std::next(context().previousNode);

    movePreviousCalculated(node, true);

    return CompletionRecord::Normal();
}

auto Interpreter::execute_STM_Expression(Parser::ParseNode node) -> CompletionRecord
{
    if(context().previousNode == node.parent()){
        context().currentNode = node.begin();
        return CompletionRecord::Normal();
    }

    movePreviousCalculated(node);

    return CompletionRecord::Normal();
}

auto Interpreter::execute_STM_Block(Parser::ParseNode node) -> CompletionRecord
{
    if(context().previousNode == node.parent()){
        if(!node.empty()){
            context().currentNode = node.begin();
        }
        return CompletionRecord::Normal();
    }
    auto nextNode = std::next(context().previousNode);
    if(nextNode != node.end()){
        context().currentNode = nextNode;
    } else {
        movePreviousCalculated(node);
    }
    return CompletionRecord::Normal();
}

auto Interpreter::execute_OPR_Grouping(Parser::ParseNode node) -> CompletionRecord
{
    if(context().previousNode == node.parent()){
        if(node.empty()){
            return CompletionRecord::Normal();
        }
        context().currentNode = node.begin();
        return CompletionRecord::Normal();
    }
    auto nextNode = std::next(context().previousNode);
    if(nextNode != node.end()){
        context().currentNode = nextNode;
        return CompletionRecord::Normal();
    }
    movePreviousCalculated(node);
    return CompletionRecord::Normal();
}

auto Interpreter::execute_OPR_JsonObject(Parser::ParseNode node) -> CompletionRecord
{
    if(context().previousNode == node.parent()){
        if(node.empty()){
            return CompletionRecord::Normal(std::unordered_map<std::string, var>{});
        }
        context().currentNode = node.begin();
        return CompletionRecord::Normal();
    }
    auto nextNode = std::next(context().previousNode);
    if(nextNode != node.end()){
        context().currentNode = nextNode;
        return CompletionRecord::Normal();
    }
    std::unordered_map<std::string, var> obj;
    for(auto n = node.begin(); n != nextNode; ++n){
        auto name = context().calculated.extract(n);
        auto value = context().calculated.extract(++n);
        obj.insert_or_assign(name.mapped().to_string(), std::move(value.mapped()));
    }
    return CompletionRecord::Normal(std::move(obj));
}

auto Interpreter::execute_OPR_MemberAccess(Parser::ParseNode node) -> CompletionRecord
{
    if(context().previousNode == node.parent()){
        context().currentNode = node.begin();
        return CompletionRecord::Normal();
    }
    if(context().previousNode == node.begin()){
        context().currentNode = std::next(context().previousNode);
        return CompletionRecord::Normal();
    }
    if(auto* opr = std::get_if<Parser::Operation>(&*node.parent());
       opr && isAssignmentOPR(*opr)
       && node.parent().begin() == node){
        return CompletionRecord::Normal();
    }
    auto* memberPtr = resolveMemberAccessNode(node);
    if(!memberPtr){
        return {CompletionRecord::Type::Throw, "ReferenceError", {}};
    }
    context().calculated.erase(node);
    if(!memberPtr->is_undefined()){
        context().calculated.try_emplace(node, *memberPtr);
    }
    return CompletionRecord::Normal(*memberPtr);
}

auto Interpreter::execute_OPR_Call(Parser::ParseNode node) -> CompletionRecord
{
    if(context().previousNode == node.parent()){
        context().currentNode = node.begin();
        return CompletionRecord::Normal();
    }
    auto nextNode = std::next(context().previousNode);
    if(nextNode != node.end()){
        context().currentNode = nextNode;
        return CompletionRecord::Normal();
    }
    std::vector<var> args;
    args.reserve(node.children() - 1);
    for(auto childNode = std::next(node.begin()); childNode != nextNode; ++childNode){
        args.push_back(context().calculated.extract(childNode).mapped());
    }
    auto lhs = context().calculated.extract(node.begin());
    try{
        return CompletionRecord::Normal(lhs.mapped().operator()(std::move(args)));
    }catch(unavailable_operation&){
        return {CompletionRecord::Type::Throw, "TypeError", {}};
    }
}

template<var(*operatorPtr)(var const&, var const&)>
auto Interpreter::execute_OPR_BinaryOperation(Parser::ParseNode node) -> CompletionRecord
{
    if(context().previousNode == node.parent()){
        context().currentNode = node.begin();
        return CompletionRecord::Normal();
    }
    if(context().previousNode == node.begin()){
        context().currentNode = std::next(node.begin());
        return CompletionRecord::Normal();
    }
    auto lhs = context().calculated.extract(node.begin());
    auto rhs = context().calculated.extract(std::next(node.begin()));
    return CompletionRecord::Normal((*operatorPtr)(lhs.mapped(), rhs.mapped()));
}

template<var&(var::*operatorPtr)(var const&)>
auto Interpreter::execute_OPR_AssignmentOperation(Parser::ParseNode node) -> CompletionRecord
{
    if(context().previousNode == node.parent()){
        auto lhsNode = node.begin();
        if(std::holds_alternative<Parser::VarUse>(*lhsNode)){
            context().currentNode = std::next(lhsNode);
            return CompletionRecord::Normal();
        }
        if(auto* operation = std::get_if<Parser::Operation>(&*lhsNode)){
            if(*operation == Parser::Operation::OPR_MemberAccess){
                context().currentNode = lhsNode;
                return CompletionRecord::Normal();
            }
            if(*operation == Parser::Operation::OPR_JsonObject){
                context().currentNode = std::next(lhsNode);
                return CompletionRecord::Normal();
            }
            if(*operation == Parser::Operation::OPR_ArrayObject){
                context().currentNode = std::next(lhsNode);
                return CompletionRecord::Normal();
            }
        }
        throw std::invalid_argument("Expected VarUse or Operation(OPR_MemberAccess|OPR_JsonObject|OPR_ArrayObject) as LeftHandSideExpression in OPR_Assignment");
    }
    //If lhs was OPR_MemberAccess, evaluate rhs
    if(context().previousNode == node.begin()){
        context().currentNode = std::next(context().previousNode);
        return CompletionRecord::Normal();
    }

    auto rhs = context().calculated.extract(context().previousNode);

    auto lhsNode = node.begin();
    if(auto* varUse = std::get_if<Parser::VarUse>(&*lhsNode)){
        auto lshPtr = resolveBinding(varUse->name);
        if(!lshPtr){
            return {CompletionRecord::Type::Throw, "ReferenceError", {}};
        }
        ((*lshPtr).*(operatorPtr))(rhs.mapped());
        return CompletionRecord::Normal(*lshPtr);
    }
    if(auto* operation = std::get_if<Parser::Operation>(&*lhsNode)){
        if(*operation == Parser::Operation::OPR_MemberAccess){
            auto lshPtr = resolveMemberAccessNode(lhsNode);
            if(!lshPtr){
                return {CompletionRecord::Type::Throw, "ReferenceError", {}};
            }
            ((*lshPtr).*(operatorPtr))(rhs.mapped());
            return CompletionRecord::Normal(*lshPtr);
        }
        if(*operation == Parser::Operation::OPR_JsonObject){
            throw unimplemented_error("Object-decomposition");
        }
        if(*operation == Parser::Operation::OPR_ArrayObject){
            throw unimplemented_error("Array-decomposition");
        }
    }

    throw std::logic_error("Unreachable code line was reached!");
}

auto Interpreter::resolveBinding(std::string const& name, var environment) -> var*
{
    if(environment.is_undefined()){
        environment = context().environment;
    }
    return &environment[name];
}

auto Interpreter::resolveMemberAccessNode(Parser::ParseNode node) -> var*
{
    auto lhs = context().calculated.extract(node.begin());
    auto rhs = context().calculated.extract(std::next(node.begin()));
    if(lhs.empty() || rhs.empty()){
        return nullptr;
    }
    lhs.key() = node;
    context().calculated.erase(node);
    auto irt = context().calculated.insert(std::move(lhs));
    return &irt.position->second[rhs.mapped()];
}

constexpr bool Interpreter::isAssignmentOPR(Parser::Operation opr) const
{
    return opr == Parser::Operation::OPR_Assignment
        || opr == Parser::Operation::OPR_AdditionAssignment
        || opr == Parser::Operation::OPR_SubtractAssignment
        || opr == Parser::Operation::OPR_MultiplicationAssignment
        || opr == Parser::Operation::OPR_DivisionAssignment
        || opr == Parser::Operation::OPR_RemainderAssignment
        || opr == Parser::Operation::OPR_LeftShiftAssignment
        || opr == Parser::Operation::OPR_RightShiftAssignment
        || opr == Parser::Operation::OPR_UnsignedRightShiftAssignment
        || opr == Parser::Operation::OPR_BitwiseANDAssignment
        || opr == Parser::Operation::OPR_BitwiseXORAssignment
        || opr == Parser::Operation::OPR_BitwiseORAssignment;
}

var Interpreter::movePreviousCalculated(Parser::ParseNode node, bool replaceIfAlreadyExists)
{
    auto valueNodeHandle = context().calculated.extract(context().previousNode);
    if(!valueNodeHandle){
        return {};
    }
    valueNodeHandle.key() = node;
    if(replaceIfAlreadyExists){
        context().calculated.erase(node);
    }
    auto irt = context().calculated.insert(std::move(valueNodeHandle));
    return irt.position->second;
}
