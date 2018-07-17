#include "Parser.h"

#define TRICKSV(name) TRICKSV_1(#name)
#define TRICKSV_1(name) name ## sv

#define OPERATIONSTR(name) {Operation::OPR_##name, TRICKSV(name) }

const std::unordered_map<Parser::Operation, std::string_view> Parser::OperationStr =
{
    OPERATIONSTR(Grouping),
    OPERATIONSTR(JsonObject),
    OPERATIONSTR(MemberAccess),
    OPERATIONSTR(New),
    OPERATIONSTR(Call),
    OPERATIONSTR(PostfixIncrement),
    OPERATIONSTR(PostfixDecrement),
    OPERATIONSTR(PrefixIncrement),
    OPERATIONSTR(PrefixDecrement),
    OPERATIONSTR(LogicalNot),
    OPERATIONSTR(BitwiseNot),
    OPERATIONSTR(UnaryPlus),
    OPERATIONSTR(UnaryNegation),
    OPERATIONSTR(Typeof),
    OPERATIONSTR(Void),
    OPERATIONSTR(Delete),
    OPERATIONSTR(Multiplication),
    OPERATIONSTR(Division),
    OPERATIONSTR(Remainder),
    OPERATIONSTR(Addition),
    OPERATIONSTR(Substraction),
    OPERATIONSTR(BitwiseLeftShift),
    OPERATIONSTR(BitwiseRightShift),
    OPERATIONSTR(BitwiseUnsignedRightShift),
    OPERATIONSTR(LessThan),
    OPERATIONSTR(LessThanOrEqual),
    OPERATIONSTR(GreaterThan),
    OPERATIONSTR(GreaterThanOrEqual),
    OPERATIONSTR(In),
    OPERATIONSTR(InstanceOf),
    OPERATIONSTR(Equality),
    OPERATIONSTR(InEquality),
    OPERATIONSTR(StrictEquality),
    OPERATIONSTR(StrictInequality),
    OPERATIONSTR(BitwiseAND),
    OPERATIONSTR(BitwiseXOR),
    OPERATIONSTR(BitwiseOR),
    OPERATIONSTR(LogicalAND),
    OPERATIONSTR(LogicalOR),
    OPERATIONSTR(Conditional),
    OPERATIONSTR(Assignment),
    OPERATIONSTR(AdditionAssignment),
    OPERATIONSTR(SubstractAssignment),
    OPERATIONSTR(MultiplicationAssignment),
    OPERATIONSTR(DivisionAssignment),
    OPERATIONSTR(RemainderAssignment),
    OPERATIONSTR(LeftShiftAssignment),
    OPERATIONSTR(RightShiftAssignment),
    OPERATIONSTR(UnsignedRightShiftAssignment),
    OPERATIONSTR(BitwiseANDAssignment),
    OPERATIONSTR(BitwiseXORAssignment),
    OPERATIONSTR(BitwiseORAssignment),
    OPERATIONSTR(Yield),
    OPERATIONSTR(Spread),
    OPERATIONSTR(Comma),
};


Parser::Parser(Lexer& source):
    m_source(source)
{

}

auto Parser::parse() -> ParseTree
{
    ParseTree tree(Statement::STM_TranslationUnit);

    while(!m_source.eof()){
        parse_Statement(tree);
    }

    return tree;
}

bool Parser::parse_Statement(ParseNode tree)
{
    if(parse_StatementBlock(tree)
       || parse_StatementIf(tree)
       || parse_StatementExpression(tree)){
        return true;
    }
    return false;
}

bool Parser::parse_StatementBlock(ParseNode tree)
{
    if(lex_expect_optional(Lexer::Punctuator::PCT_brace_left)){
        auto block = tree.append(Statement::STM_Block);
        while(parse_Statement(block))
        {}
        lex_expect(Lexer::Punctuator::PCT_brace_right);
        return true;
    }
    return false;
}

bool Parser::parse_StatementExpression(ParseNode tree)
{
    ParseTree expression(Statement::STM_Expression);
    if(parse_varDecl(expression) || parse_evaluationExpression(expression, 0)){
        lex_expect(Lexer::Punctuator::PCT_semicolon);
        tree.append(std::move(expression));
        return true;
    }

    return false;
}

bool Parser::parse_StatementIf(ParseNode tree)
{
    if(!lex_expect_optional(Lexer::Keyword::KWD_if)){
        return false;
    }
    lex_expect(Lexer::Punctuator::PCT_parenthese_left);
    auto ifStm = tree.append(Statement::STM_If);
    if(!parse_evaluationExpression(ifStm, 0)){
        expected("EvaluationExpression"s, lex());
    }
    lex_expect(Lexer::Punctuator::PCT_parenthese_right);
    if(!parse_Statement(ifStm)){
        expected("Statement"s, lex());
    }
    parse_StatementElse(ifStm);
    return true;
}

bool Parser::parse_StatementElse(ParseNode tree)
{
    if(!lex_expect_optional(Lexer::Keyword::KWD_else)){
        return false;
    }
    if(!parse_Statement(tree)){
        expected("Statement"s, lex());
    }
    return true;
}

bool Parser::parse_varDecl(ParseNode tree)
{
    if(!lex_expect_optional(Lexer::Keyword::KWD_var)){
        return false;
    }

    Lexem name = lex();
    auto* nameIdent = std::get_if<Lexer::Identifier>(&name);
    if(!nameIdent){
        expected(Lexer::Identifier{}, name);
    }
    auto varDeclNode = tree.append(ParseResult{VarDecl{std::move(*nameIdent)}});

    if(lex_expect_optional(Lexer::Punctuator::PCT_equal)){
        parse_evaluationExpression(varDeclNode, 0);
    }

    return true;
}

bool Parser::parse_evaluationExpression(ParseNode tree, int opr_precedence)
{
    bool ret = false;
    if(parse_Literal(tree) || parse_varUse(tree)){
        ret = true;
        opr_precedence = 0x100;
    }
    while(parse_Operation(tree, opr_precedence)){
        ret = true;
        opr_precedence = 0x100;
    }
    return ret;
}

bool Parser::parse_Operation(ParseNode tree, int opr_precedence)
{
    if(opr_precedence == 0){
        if(lex_expect_optional(Lexer::Punctuator::PCT_parenthese_left)){
            auto operation = tree.append(Operation::OPR_Grouping);
            if(!parse_evaluationExpression(operation, 0)){
                expected("EvaluationExpression"s, lex());
            }
            lex_expect(Lexer::Punctuator::PCT_parenthese_right);
            return true;
        }
    }

    if(!tree.empty()){
        if(lex_expect_optional(Lexer::Punctuator::PCT_point)){
            auto previous = tree.last_child();
            previous.wrap(Operation::OPR_MemberAccess);
            Lexem name = lex();
            auto* nameIdent = std::get_if<Lexer::Identifier>(&name);
            if(!nameIdent){
                expected(Lexer::Identifier{}, name);
            }
            previous.append(std::move(*nameIdent));
            return true;
        }
        if(lex_expect_optional(Lexer::Punctuator::PCT_bracket_left)){
            auto previous = tree.last_child();
            previous.wrap(Operation::OPR_MemberAccess);
            if(!parse_evaluationExpression(previous, 0)){
                expected("EvaluationExpression"s, lex());
            }
            lex_expect(Lexer::Punctuator::PCT_bracket_right);
            return true;
        }

        if(lex_expect_optional(Lexer::Punctuator::PCT_parenthese_left)){
            auto previous = previousWrapPrecedence(tree, Operation::OPR_Call);
            if(parse_evaluationExpression(previous, 0)){
                while(lex_expect_optional(Lexer::Punctuator::PCT_comma)){
                    if(!parse_evaluationExpression(previous, 0)){
                        expected("EvaluationExpression"s, lex());
                    }
                }
            }
            lex_expect(Lexer::Punctuator::PCT_parenthese_right);
            return true;
        }

        if(lex_expect_optional(Lexer::Punctuator::PCT_double_plus)){
            previousWrapPrecedence(tree, Operation::OPR_PostfixIncrement);
            return true;
        }
        if(lex_expect_optional(Lexer::Punctuator::PCT_double_minus)){
            previousWrapPrecedence(tree, Operation::OPR_PostfixDecrement);
            return true;
        }
    }

    if(parse_prefixUnaryOperation(Lexer::Punctuator::PCT_double_plus,  Operation::OPR_PrefixIncrement, tree, opr_precedence)
    || parse_prefixUnaryOperation(Lexer::Punctuator::PCT_double_minus, Operation::OPR_PrefixDecrement, tree, opr_precedence)
    || parse_prefixUnaryOperation(Lexer::Punctuator::PCT_not,          Operation::OPR_LogicalNot,      tree, opr_precedence)
    || parse_prefixUnaryOperation(Lexer::Punctuator::PCT_tilde,        Operation::OPR_BitwiseNot,      tree, opr_precedence)
    || parse_prefixUnaryOperation(Lexer::Punctuator::PCT_plus,         Operation::OPR_UnaryPlus,       tree, opr_precedence)
    || parse_prefixUnaryOperation(Lexer::Punctuator::PCT_minus,        Operation::OPR_UnaryNegation,   tree, opr_precedence)
    || parse_prefixUnaryOperation(Lexer::Keyword::KWD_typeof,          Operation::OPR_Typeof,          tree, opr_precedence)
    || parse_prefixUnaryOperation(Lexer::Keyword::KWD_void,            Operation::OPR_Void,            tree, opr_precedence)
    || parse_prefixUnaryOperation(Lexer::Keyword::KWD_delete,          Operation::OPR_Delete,          tree, opr_precedence)){
        return true;
    }

    if(!tree.empty()){
        if(auto opr = std::get_if<Operation>(&*tree);
           opr_precedence == 0 || !opr || precedence(*opr) < precedence(Operation::OPR_Multiplication)){
            if(lex_expect_optional(Lexer::Punctuator::PCT_times)){
                auto previous = previousWrapPrecedence(tree, Operation::OPR_Multiplication);
                if(!parse_evaluationExpression(previous, 0)){
                    expected("EvaluationExpression"s, lex());
                }
                return true;
            }
            if(lex_expect_optional(Lexer::Punctuator::PCT_double_times)){
                auto previous = previousWrapPrecedence(tree, Operation::OPR_Exponentiation);
                if(!parse_evaluationExpression(previous, 0)){
                    expected("EvaluationExpression"s, lex());
                }
                return true;
            }
            if(lex_expect_optional(Lexer::Punctuator::PCT_divided)){
                auto previous = previousWrapPrecedence(tree, Operation::OPR_Division);
                if(!parse_evaluationExpression(previous, 0)){
                    expected("EvaluationExpression"s, lex());
                }
                return true;
            }
            if(lex_expect_optional(Lexer::Punctuator::PCT_modulo)){
                auto previous = previousWrapPrecedence(tree, Operation::OPR_Remainder);
                if(!parse_evaluationExpression(previous, 0)){
                    expected("EvaluationExpression"s, lex());
                }
                return true;
            }
        }

        if(auto opr = std::get_if<Operation>(&*tree);
           opr_precedence == 0 || !opr || precedence(*opr) < precedence(Operation::OPR_Addition)){
            if(lex_expect_optional(Lexer::Punctuator::PCT_plus)){
                auto previous = previousWrapPrecedence(tree, Operation::OPR_Addition);
                if(!parse_evaluationExpression(previous, 0)){
                    expected("EvaluationExpression"s, lex());
                }
                return true;
            }
            if(lex_expect_optional(Lexer::Punctuator::PCT_minus)){
                auto previous = previousWrapPrecedence(tree, Operation::OPR_Substraction);
                if(!parse_evaluationExpression(previous, 0)){
                    expected("EvaluationExpression"s, lex());
                }
                return true;
            }
        }
    }

    // parse postfix

    return false;
}

auto Parser::previousWrapPrecedence(ParseNode tree, Operation operation) const -> ParseNode
{
    auto previous = tree.last_child();
    Operation* opr = nullptr;
    while((opr = std::get_if<Operation>(&*previous))
          && precedence(*opr) < precedence(operation)) {
        previous = previous.last_child();
    }
    previous.wrap(operation);
    return previous;
}

bool Parser::parse_prefixUnaryOperation(Lexem lxm, Operation opr, ParseNode tree, int opr_precedence)
{
    if(opr_precedence > 0){
        return false;
    }
    if(lex_expect_optional(lxm)){
        auto operation = tree.append(opr);
        if(!parse_evaluationExpression(operation, 0)){
            expected("EvaluationExpression"s, lex());
        }
        return true;
    }
    return false;
}

bool Parser::parse_Literal(ParseNode tree)
{
    Lexem lxm = lex();
    if(auto* lit = std::get_if<Lexer::Literal>(&lxm); lit){
        tree.append(ParseResult{std::move(*lit)});
        return true;
    }
    if(auto* lit = std::get_if<Lexer::Keyword>(&lxm); lit){
        switch(*lit){
        case Lexer::Keyword::KWD_true:
            tree.append(true);
            return true;
        case Lexer::Keyword::KWD_false:
            tree.append(false);
            return true;
        case Lexer::Keyword::KWD_null:
            tree.append(nullptr);
            return true;
        default:
            break;
        }
    }
    lex_putback(std::move(lxm));
    return false;
}

bool Parser::parse_varUse(ParseNode tree)
{
    Lexem name = lex();
    if(auto* nameIdent = std::get_if<Lexer::Identifier>(&name); nameIdent){
        tree.append(ParseResult{VarUse{std::move(*nameIdent)}});
        return true;
    }
    lex_putback(std::move(name));
    return false;
}



auto Parser::lex() -> Lexem
{
    if(m_current_unit.empty()){
        return m_source.lex();
    }
    Lexem lxm = m_current_unit.back();
    m_current_unit.pop_back();
    return lxm;
}

void Parser::lex_putback(Lexem&& lxm)
{
    m_current_unit.push_back(std::move(lxm));
}


bool Parser::lex_expect_optional(Lexem exp)
{
    Lexem lxm = lex();
    if(lxm == exp){
        return true;
    }
    lex_putback(std::move(lxm));
    return false;
}

void Parser::lex_expect(Lexem exp)
{
    Lexem lxm = lex();
    if(!(lxm == exp)){
        expected(exp, lxm);
    }
}

void Parser::expected(Lexem expected, Lexem unexpected) noexcept(false)
{
    std::ostringstream oss;
    oss << expected;
    std::string expStr = oss.str();
    oss.str("");
    oss << unexpected;
    std::string unexpStr = oss.str();
    throw std::invalid_argument("Expected " + expStr + ", but encountered " + unexpStr + ".");
}

