#include "Interpreter.h"

#include <cassert>
#include <iostream>

Interpreter::Interpreter()
{
    //ctor
}

void Interpreter::feed(Parser::ParseTree tree)
{
    m_parseTrees.push_back(std::make_unique<Parser::ParseTree>(std::move(tree)));
    ExecutionContext ctx {
        Realm{},
        var{},
        m_globalEnvironment,
        m_parseTrees.back()->root(),
        m_parseTrees.back()->root(),
        m_parseTrees.back()->root(),
        {}
    };
    m_executionStack.push(std::move(ctx));
}

var Interpreter::execute()
{
    auto& ctx = m_executionStack.top();
    ctx.currentNode = ctx.code;
    ctx.previousNode = ctx.currentNode;
    CompletionRecord cr;
    while(!m_executionStack.empty()){
        cr = execute_step();
    }
    return cr.value;
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
        if(!m_executionStack.empty()){
            auto& parentCtx = m_executionStack.top();
            parentCtx.calculated.insert_or_assign(parentCtx.previousNode, cr.value);
        }
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
    assert(std::holds_alternative<Parser::Literal>(nodeVal));
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
    case Statement::STM_If:
        return execute_STM_If(node);
    case Statement::STM_Return:
        return execute_STM_Return(node);
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
    case Operation::OPR_Function:
        return execute_OPR_Function(node);
    case Operation::OPR_MemberAccess:
        return execute_OPR_MemberAccess(node);
//    case Operation::OPR_New:
//        return execute_OPR_New(node);
    case Operation::OPR_Call:
        return execute_OPR_Call(node);
    case Operation::OPR_PostfixIncrement:
        return execute_OPR_PostfixAssignmentOperation<&var::operator++ >(node);
    case Operation::OPR_PostfixDecrement:
        return execute_OPR_PostfixAssignmentOperation<&var::operator-- >(node);
    case Operation::OPR_PrefixIncrement:
        return execute_OPR_PrefixAssignmentOperation<&var::operator++ >(node);
    case Operation::OPR_PrefixDecrement:
        return execute_OPR_PrefixAssignmentOperation<&var::operator-- >(node);
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
    case Operation::OPR_LessThan:
        return execute_OPR_BinaryOperation<operator< >(node);
    case Operation::OPR_LessThanOrEqual:
        return execute_OPR_BinaryOperation<operator<= >(node);
    case Operation::OPR_GreaterThan:
        return execute_OPR_BinaryOperation<operator> >(node);
    case Operation::OPR_GreaterThanOrEqual:
        return execute_OPR_BinaryOperation<operator>= >(node);
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
    case Operation::OPR_LogicalAND:
        return execute_OPR_LogicalAND(node);
    case Operation::OPR_LogicalOR:
        return execute_OPR_LogicalOR(node);
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

    var result = movePreviousCalculated(node, true);

    if(context().currentNode == node.end()) {
        return {CompletionRecord::Type::Return, result, {}};
    }
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

auto Interpreter::execute_STM_If(Parser::ParseNode node) -> CompletionRecord
{
    if(context().previousNode == node.parent()){
        context().currentNode = node.begin();
        return CompletionRecord::Normal();
    }
    if(context().previousNode == node.begin()){
        var condition = context().calculated.extract(node.begin()).mapped();
        if(condition.to_bool() == true){
            context().currentNode = std::next(node.begin(), 1);
        } else if(node.children() == 3) {
            context().currentNode = std::next(node.begin(), 2);
        }
        return CompletionRecord::Normal();
    }
    movePreviousCalculated(node);
    return CompletionRecord::Normal();
}

auto Interpreter::execute_STM_Return(Parser::ParseNode node) -> CompletionRecord
{
    if(context().previousNode == node.parent()){
        if(node.empty()){
            return CompletionRecord{CompletionRecord::Type::Return, var::undefined, {}};
        }
        context().currentNode = node.begin();
        return CompletionRecord::Normal();
    }
    auto rhs = context().calculated.extract(node.begin());
    return CompletionRecord{CompletionRecord::Type::Return, rhs ? rhs.mapped() : var::undefined, {}};
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

auto Interpreter::execute_OPR_Function(Parser::ParseNode node) -> CompletionRecord
{
    auto funcName = *node.begin();
    auto funcCode = std::find_if(std::next(node.begin()), node.end(), [](auto& x){
        auto* stmPtr = std::get_if<Parser::Statement>(&x);
        return stmPtr && *stmPtr == Parser::Statement::STM_Block;
    });
    std::vector<std::string> funcParams;
    for(auto it = std::next(node.begin()); it != funcCode; ++it){
        funcParams.push_back(std::get<Parser::VarDecl>(*it).name);
    }
//    std::vector<std::string> captureList = computeCaptureList(funcCode, std::get<Parser::Literal>(funcName).to_string(), funcParams);
//    std::unordered_map<std::string, var> captureValues;
//    for(auto& captName : captureList){
//        var* captValue = resolveBinding(captName, context().environment);
//        if(captValue == nullptr){
//            return {CompletionRecord::Type::Throw, "CaptureError", {}};
//        }
//        captureValues.emplace(captName, *captValue);
//    }

    Parser::ParseTree funcTree{Parser::Statement::STM_TranslationUnit};
    funcTree.root().append_copy(funcCode);

    return CompletionRecord::Normal(var{[funcTree = std::move(funcTree), funcParams, capturedEnv = context().environment, this](std::vector<var> arguments) mutable{
        var environment{{}, capturedEnv};

        size_t i = 0;
        for(auto& param : funcParams){
            environment[param] = arguments.size() > i ? arguments[i++] : var::undefined;
        }
        // make a copy of the code tree
        m_parseTrees.push_back(std::make_unique<Parser::ParseTree>(funcTree));
        auto funcCode = m_parseTrees.back()->root();
        ExecutionContext ctx {
            Realm{},
            var{},
            environment,
            funcCode,
            funcCode,
            funcCode,
            {}
        };
        m_executionStack.push(std::move(ctx));
        return var::undefined;
    }});
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
        auto argVarNode = context().calculated.extract(childNode);
        args.push_back(argVarNode ? argVarNode.mapped() : var::undefined);
    }
    auto lhs = context().calculated.extract(node.begin());
    if(lhs.empty()){
        return {CompletionRecord::Type::Throw, "ReferenceError", {}};
    }
    try{
        return CompletionRecord::Normal(lhs.mapped().operator()(std::move(args)));
    }catch(unavailable_operation&){
        return {CompletionRecord::Type::Throw, "TypeError", {}};
    }
}

auto Interpreter::execute_OPR_LogicalAND(Parser::ParseNode node) -> CompletionRecord
{
    if(context().previousNode == node.parent()){
        context().currentNode = node.begin();
        return CompletionRecord::Normal();
    }
    if(context().previousNode == node.begin()){
        auto lhs = context().calculated.extract(node.begin());
        if(!lhs){
            return CompletionRecord::Normal();
        }
        if(lhs.mapped().to_bool() == false){
            return CompletionRecord::Normal(lhs.mapped());
        }
        context().currentNode = std::next(node.begin());
        return CompletionRecord::Normal();
    }
    auto rhs = context().calculated.extract(std::next(node.begin()));
    return CompletionRecord::Normal(rhs ? rhs.mapped() : var{});
}

auto Interpreter::execute_OPR_LogicalOR(Parser::ParseNode node) -> CompletionRecord
{
    if(context().previousNode == node.parent()){
        context().currentNode = node.begin();
        return CompletionRecord::Normal();
    }
    if(context().previousNode == node.begin()){
        auto lhs = context().calculated.extract(node.begin());
        if(!lhs){
            return CompletionRecord::Normal();
        }
        if(lhs.mapped().to_bool() == true){
            return CompletionRecord::Normal(lhs.mapped());
        }
        context().currentNode = std::next(node.begin());
        return CompletionRecord::Normal();
    }
    auto rhs = context().calculated.extract(std::next(node.begin()));
    return CompletionRecord::Normal(rhs ? rhs.mapped() : var{});
}

template<auto operatorPtr>
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
    return CompletionRecord::Normal((*operatorPtr)(lhs ? lhs.mapped() : var{}, rhs ? rhs.mapped() : var{}));
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
        ((*lshPtr).*(operatorPtr))(rhs ? rhs.mapped() : var{});
        return CompletionRecord::Normal(*lshPtr);
    }
    if(auto* operation = std::get_if<Parser::Operation>(&*lhsNode)){
        if(*operation == Parser::Operation::OPR_MemberAccess){
            auto lshPtr = resolveMemberAccessNode(lhsNode);
            if(!lshPtr){
                return {CompletionRecord::Type::Throw, "ReferenceError", {}};
            }
            ((*lshPtr).*(operatorPtr))(rhs ? rhs.mapped() : var{});
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

template<var(var::*operatorPtr)(int)>
auto Interpreter::execute_OPR_PostfixAssignmentOperation(Parser::ParseNode node) -> CompletionRecord
{
    auto lhsNode = node.begin();
    if(context().previousNode == node.parent()){
        if(auto operation = std::get_if<Parser::Operation>(&*lhsNode);
                operation && *operation == Parser::Operation::OPR_MemberAccess){
            context().currentNode = lhsNode;
            return CompletionRecord::Normal();
        }
    }

    var* lshPtr = nullptr;

    if(auto* varUse = std::get_if<Parser::VarUse>(&*lhsNode)){
        lshPtr = resolveBinding(varUse->name);
    } else if(auto operation = std::get_if<Parser::Operation>(&*lhsNode);
                operation && *operation == Parser::Operation::OPR_MemberAccess){
        lshPtr = resolveMemberAccessNode(lhsNode);
    } else {
        throw std::invalid_argument("Expected VarUse or Operation(OPR_MemberAccess) as LeftHandSideExpression in unary assignment");
    }

    if(!lshPtr){
        return {CompletionRecord::Type::Throw, "ReferenceError", {}};
    }
    var oldValue = *lshPtr;
    ((*lshPtr).*(operatorPtr))(0);
    return CompletionRecord::Normal(oldValue);
}

template<var&(var::*operatorPtr)()>
auto Interpreter::execute_OPR_PrefixAssignmentOperation(Parser::ParseNode node) -> CompletionRecord
{
    auto lhsNode = node.begin();
    if(context().previousNode == node.parent()){
        if(auto operation = std::get_if<Parser::Operation>(&*lhsNode);
                operation && *operation == Parser::Operation::OPR_MemberAccess){
            context().currentNode = lhsNode;
            return CompletionRecord::Normal();
        }
    }

    var* lshPtr = nullptr;

    if(auto* varUse = std::get_if<Parser::VarUse>(&*lhsNode)){
        lshPtr = resolveBinding(varUse->name);
    } else if(auto operation = std::get_if<Parser::Operation>(&*lhsNode);
                   operation && *operation == Parser::Operation::OPR_MemberAccess){
        lshPtr = resolveMemberAccessNode(lhsNode);
    } else {
        throw std::invalid_argument("Expected VarUse or Operation(OPR_MemberAccess) as LeftHandSideExpression in unary assignment");
    }

    if(!lshPtr){
        return {CompletionRecord::Type::Throw, "ReferenceError", {}};
    }
    ((*lshPtr).*(operatorPtr))();
    return CompletionRecord::Normal(*lshPtr);
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
        || opr == Parser::Operation::OPR_BitwiseORAssignment
        || opr == Parser::Operation::OPR_PostfixIncrement
        || opr == Parser::Operation::OPR_PostfixDecrement
        || opr == Parser::Operation::OPR_PrefixIncrement
        || opr == Parser::Operation::OPR_PrefixDecrement;
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

auto Interpreter::computeCaptureList(Parser::ParseNode funcCode, std::string const& funcName, std::vector<std::string> funcParams) const -> std::vector<std::string>
{
    std::vector<std::string> captureList;
    std::vector<std::vector<std::string>> knownNames;
    funcParams.push_back(funcName);
    knownNames.push_back(funcParams);

    auto isKnown = [&captureList, &knownNames](std::string const& name){
        if(std::find(std::begin(captureList), std::end(captureList), name) != std::end(captureList)){
            return true;
        }
        for(auto &kn : knownNames){
            if(std::find(std::begin(kn), std::end(kn), name) != std::end(kn)){
                return true;
            }
        }
        return false;
    };

    auto computeFuncKnownList = [](Parser::ParseNode funcNode){
        std::vector<std::string> knownNames;
        knownNames.push_back(std::get<Parser::Literal>(*funcNode.begin()).to_string());
        auto funcCode = funcNode.last_child();
        for(auto it = std::next(funcNode.begin()); it != funcCode; ++it){
            knownNames.push_back(std::get<Parser::VarDecl>(*it).name);
        }
        return knownNames;
    };

    auto funcEnd = funcCode.end();
    for(auto node = funcCode.begin(); node != funcEnd;){
        if(auto* vd = std::get_if<Parser::VarDecl>(&*node); vd){
            knownNames.rbegin()->push_back(vd->name);
        } else if(auto* vu = std::get_if<Parser::VarUse>(&*node); vu){
            if(!isKnown(vu->name)){
                captureList.push_back(vu->name);
            }
        } else if(auto* fo = std::get_if<Parser::Operation>(&*node); fo && *fo == Parser::Operation::OPR_Function){
            knownNames.push_back(computeFuncKnownList(node));
            node = node.last_child();
        }

        if(node.children() > 0){
            node = node.begin();
        } else {
            auto nextNode = std::next(node);
            if(nextNode == funcEnd){ break; }
            for(auto nodeParent = node.parent(), nextNodeParent = nextNode.parent();
                    nodeParent != nextNodeParent; nodeParent = nodeParent.parent()){
                if(auto* fo = std::get_if<Parser::Operation>(&*nodeParent); fo && *fo == Parser::Operation::OPR_Function){
                    knownNames.pop_back();
                }
            }
            node = nextNode;
        }
    }

    return captureList;
}


namespace {

template<typename Tag, typename Tag::type M>
struct Rob {
    friend typename Tag::type get(Tag) {
        return M;
    }
};

struct ParseNodeRobber {
    typedef std::vector<std::pair<int, Parser::ParseResult>> * Parser::ParseNode::*type;
    friend type get(ParseNodeRobber);
};
template struct Rob<ParseNodeRobber, &Parser::ParseNode::m_tree>;

struct ParseNodeIndexRobber {
    typedef std::vector<std::pair<int, Parser::ParseResult>>::difference_type Parser::ParseNode::*type;
    friend type get(ParseNodeIndexRobber);
};
template struct Rob<ParseNodeIndexRobber, &Parser::ParseNode::m_index>;


template<class T>
struct StackInspector: public std::stack<T>
{
    using std::stack<T>::c;
};
template<class T>
static StackInspector<T> const& inspect(std::stack<T> const& s){ return *static_cast<StackInspector<T> const*>(&s); }

}

std::ostream& operator<<(std::ostream& out, Interpreter const& interpreter) {
    out << "=== ParseTrees ===\n";
    for(auto& pt : interpreter.m_parseTrees){
        out << *pt;
    }
    out << "=== Stack ===\n";
    for(auto& exec : inspect(interpreter.m_executionStack).c){
        out << "ExecutionContext{\n";
        out << "environment: " << exec.environment << "\n";
        auto tree = exec.code.*get(ParseNodeRobber());
        {
            int i = 0;
            for(auto& [lweight, value] : *tree){
                out << std::string(static_cast<std::string::size_type>(i), '>') << lweight << ':' << value << '\n';
                i += lweight;
            }
        }
        out << "calculated: {";
        for(auto& [parseNode, value] : exec.calculated){
            out << parseNode.*get(ParseNodeIndexRobber()) << ":" << value << ",";
        }
        out << "}\n";
        out << "}\n";
    }
    out << "GlobalEnvironment: " << interpreter.m_globalEnvironment << "\n";
    return out;
}
