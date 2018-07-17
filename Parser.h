#pragma once

#include "Lexer.h"
#include "ParseTree.h"

class Parser
{
public:
    Parser(Lexer& source);

    struct VarDecl
    {
        std::string name;
    };

    struct VarUse
    {
        std::string name;
    };

    enum class Statement
    {
        STM_TranslationUnit,
        STM_Expression,
        STM_Block,
        STM_If,
        STM_While,
        STM_For,
        STM_Return,
        STM_Throw,
        STM_Catch,
        STM_Finally,
    };

    static constexpr std::string_view StatementStr[] =
    {
        "TranslationUnit"sv,
        "Expression"sv,
        "Block"sv,
        "If"sv,
        "While"sv,
        "For"sv,
        "Return"sv,
        "Throw"sv,
        "Catch"sv,
        "Finally"sv,
    };

    enum class Operation
    {
        OPR_Grouping   = 0x1300,
        OPR_JsonObject = 0x1301,

        OPR_MemberAccess = 0x1200,
        OPR_New          = 0x1201,

        OPR_Call = 0x1100,

        OPR_PostfixIncrement = 0x1000,
        OPR_PostfixDecrement = 0x1001,

        OPR_PrefixIncrement = 0x0f00,
        OPR_PrefixDecrement = 0x0f01,
        OPR_LogicalNot      = 0x0f02,
        OPR_BitwiseNot      = 0x0f03,
        OPR_UnaryPlus       = 0x0f04,
        OPR_UnaryNegation   = 0x0f05,
        OPR_Typeof          = 0x0f06,
        OPR_Void            = 0x0f07,
        OPR_Delete          = 0x0f08,

        OPR_Multiplication = 0x0e00,
        OPR_Division       = 0x0e01,
        OPR_Remainder      = 0x0e02,

        OPR_Addition     = 0x0d00,
        OPR_Substraction = 0x0d01,

        OPR_BitwiseLeftShift          = 0x0c00,
        OPR_BitwiseRightShift         = 0x0c01,
        OPR_BitwiseUnsignedRightShift = 0x0c02,

        OPR_LessThan           = 0x0b00,
        OPR_LessThanOrEqual    = 0x0b01,
        OPR_GreaterThan        = 0x0b02,
        OPR_GreaterThanOrEqual = 0x0b03,
        OPR_In                 = 0x0b04,
        OPR_InstanceOf         = 0x0b05,

        OPR_Equality         = 0x0a00,
        OPR_InEquality       = 0x0a01,
        OPR_StrictEquality   = 0x0a02,
        OPR_StrictInequality = 0x0a03,

        OPR_BitwiseAND = 0x0900,

        OPR_BitwiseXOR = 0x0800,

        OPR_BitwiseOR = 0x0700,

        OPR_LogicalAND = 0x0600,

        OPR_LogicalOR = 0x0500,

        OPR_Conditional = 0x0400,

        OPR_Assignment                   = 0x0300,
        OPR_AdditionAssignment           = 0x0301,
        OPR_SubstractAssignment          = 0x0302,
        OPR_MultiplicationAssignment     = 0x0303,
        OPR_DivisionAssignment           = 0x0304,
        OPR_RemainderAssignment          = 0x0305,
        OPR_LeftShiftAssignment          = 0x0306,
        OPR_RightShiftAssignment         = 0x0307,
        OPR_UnsignedRightShiftAssignment = 0x0308,
        OPR_BitwiseANDAssignment         = 0x0309,
        OPR_BitwiseXORAssignment         = 0x030a,
        OPR_BitwiseORAssignment          = 0x030b,

        OPR_Yield = 0x0200,

        OPR_Spread = 0x0100,

        OPR_Comma = 0x0000//Reaaaaally not sure to include this one.

    } opr;

    static auto precedence(Operation opr){ return fys::underlying_cast(opr) / 0x100; }

    static const std::unordered_map<Operation, std::string_view> OperationStr;

    using Literal = var;

    using ParseResult = std::variant<
        VarDecl,
        VarUse,
        Statement,
        Operation,
        Literal
    >;

    using ParseTree = ::ParseTree<ParseResult>;
    using ParseNode = ParseTree::Node;

    ParseTree parse();

private:
    using Lexem = Lexer::Lexem;

    bool parse_Statement(ParseNode tree);
    bool parse_StatementBlock(ParseNode tree);
    bool parse_StatementExpression(ParseNode tree);
    bool parse_StatementIf(ParseNode tree);
    bool parse_StatementElse(ParseNode tree);
    bool parse_varDecl(ParseNode tree);
    bool parse_evaluationExpression(ParseNode tree, int opr_precedence);
    bool parse_Operation(ParseNode tree, int opr_precedence);
    bool parse_prefixUnaryOperation(Lexem lxm, Operation opr, ParseNode tree, int opr_precedence);
    bool parse_Literal(ParseNode tree);
    bool parse_varUse(ParseNode tree);

    auto previousWrapPrecedence(ParseNode tree, Operation operation) const -> ParseNode;

    Lexer& m_source;

    std::vector<Lexem> m_current_unit;

    Lexem lex();
    void lex_putback(Lexem&& lxm);

    bool lex_expect_optional(Lexem exp);
    void lex_expect(Lexem exp);

    [[noreturn]] void expected(Lexem expected, Lexem unexpected) noexcept(false);
};

inline std::ostream& operator<<(std::ostream& os, Parser::ParseResult const& parseResult)
{
    struct Visitor{
        std::ostream& os;
        void operator()(Parser::VarDecl const& pr){
            os << "VarDecl(name:" << pr.name << ")";
        }
        void operator()(Parser::VarUse const& pr){
            os << "VarUse(name:" << pr.name << ")";
        }
        void operator()(Parser::Statement const& pr){
            os << "Statement(" << std::hex << fys::underlying_cast(pr) << std::dec << ":" << Parser::StatementStr[fys::underlying_cast(pr)] << ")";
        }
        void operator()(Parser::Operation const& pr){
            os << "Operation(" << std::hex << fys::underlying_cast(pr) << std::dec << ":" << Parser::OperationStr.at(pr) << ")";
        }
        void operator()(Parser::Literal const& pr){
            os << "Literal(" << pr << ")";
        }
    };

    std::visit(Visitor{os}, parseResult);

    return os;
}
