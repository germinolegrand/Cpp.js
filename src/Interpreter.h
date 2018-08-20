#pragma once

#include "Parser.h"

class Interpreter
{
public:
    Interpreter();

    void feed(Parser::ParseTree tree);

    var execute();

    class unimplemented_error: public std::runtime_error
    {
    public:
        explicit unimplemented_error(std::string const& what_arg): runtime_error(what_arg){}
        explicit unimplemented_error(const char* what_arg): runtime_error(what_arg){}
        explicit unimplemented_error(Parser::Statement stm): runtime_error(std::string(Parser::StatementStr[fys::underlying_cast(stm)])){}
        explicit unimplemented_error(Parser::Operation opr): runtime_error(std::string(Parser::OperationStr.at(opr))){}
    };

private:
    struct CompletionRecord
    {
        enum class Type
        {
            Normal,
            Break,
            Continue,
            Return,
            Throw
        };

        Type type;
        var value;
        std::string target;

        static CompletionRecord Normal(){ return {Type::Normal, {}, {}}; }
        static CompletionRecord Normal(var v){ return {Type::Normal, std::move(v), {}}; }
    };

    struct Realm
    {

    };

    struct ExecutionContext
    {
        Realm realm;
        var function;
        var environment;
        Parser::ParseNode code;
        Parser::ParseNode currentNode;
        Parser::ParseNode previousNode;
        std::unordered_map<Parser::ParseNode, var, Parser::ParseNode::Hash> calculated;
    };

    auto execute_step() -> CompletionRecord;

    auto execute_Node(Parser::ParseNode node) -> CompletionRecord;
    auto execute_Statement(Parser::ParseNode node) -> CompletionRecord;
    auto execute_Operation(Parser::ParseNode node) -> CompletionRecord;
    auto execute_VarUse(Parser::ParseNode node) -> CompletionRecord;
    auto execute_VarDecl(Parser::ParseNode node) -> CompletionRecord;

    auto execute_STM_TranslationUnit(Parser::ParseNode node) -> CompletionRecord;
    auto execute_STM_Expression(Parser::ParseNode node) -> CompletionRecord;
    auto execute_STM_Block(Parser::ParseNode node) -> CompletionRecord;

    auto execute_OPR_Grouping(Parser::ParseNode node) -> CompletionRecord;
    auto execute_OPR_JsonObject(Parser::ParseNode node) -> CompletionRecord;
    auto execute_OPR_MemberAccess(Parser::ParseNode node) -> CompletionRecord;
    auto execute_OPR_Assignment(Parser::ParseNode node) -> CompletionRecord;

    auto resolveBinding(std::string const& name, var environment = {}) -> var*;
    auto resolveMemberAccessNode(Parser::ParseNode node) -> var*;

    var movePreviousCalculated(Parser::ParseNode node, bool replaceIfAlreadyExists = false);

    ExecutionContext& context(){ return m_executionStack.top(); }

    std::vector<Parser::ParseTree> m_parseTrees;
    std::stack<ExecutionContext> m_executionStack;
};
