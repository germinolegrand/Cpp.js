#include "Interpreter.h"

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
        var{{{"test", "Environment"}}},
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
    } else {
        throw unavailable_operation();
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
        throw unavailable_operation();
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
//    case Operation::OPR_MemberAccess:
//        return execute_OPR_MemberAccess(node);
    default:
        throw unavailable_operation();
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
    if(node.parent() == context().previousNode){
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
    if(node.parent() == context().previousNode){
        context().currentNode = node.begin();
        return CompletionRecord::Normal();
    }

    movePreviousCalculated(node);

    return CompletionRecord::Normal();
}

auto Interpreter::execute_STM_Block(Parser::ParseNode node) -> CompletionRecord
{
    if(node.parent() == context().previousNode){
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
    if(node.parent() == context().previousNode){
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
    if(node.parent() == context().previousNode){
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

var Interpreter::movePreviousCalculated(Parser::ParseNode node, bool replaceIfAlreadyExists)
{
    auto valueNodeHandle = context().calculated.extract(context().previousNode);
    if(!valueNodeHandle){
        return {};
    }
    valueNodeHandle.key() = node;
    auto irt = context().calculated.insert(std::move(valueNodeHandle));
    if(!irt.inserted && replaceIfAlreadyExists){
        context().calculated.erase(node);
        context().calculated.insert(std::move(irt.node));
    }
    return irt.position->second;
}
