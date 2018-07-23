#include "Parser.h"

#define TRICKSV(name) TRICKSV_1(#name)
#define TRICKSV_1(name) name ## sv

#define OPERATIONSTR(name) {Operation::OPR_##name, TRICKSV(name) }

const std::unordered_map<Parser::Operation, std::string_view> Parser::OperationStr =
{
    OPERATIONSTR(Grouping),
    OPERATIONSTR(JsonObject),
    OPERATIONSTR(ArrayObject),
    OPERATIONSTR(Function),
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
    OPERATIONSTR(Exponentiation),
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
    OPERATIONSTR(Inequality),
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

    while(!lex_expect_optional(Lexer::Symbol::SBL_EOF)){
        if(!parse_Statement(tree)){
            expected("Statement"s, lex());
        }
    }

    return tree;
}

bool Parser::parse_Statement(ParseNode tree)
{
    if(parse_StatementBlock(tree)
       || parse_StatementIf(tree)
       || parse_StatementWhile(tree)
       || parse_StatementDoWhile(tree)
       || parse_StatementReturn(tree)
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
        if(!expression.empty()
           && std::holds_alternative<Operation>(*expression.at(0))
           && std::get<Operation>(*expression.at(0)) == Operation::OPR_Function
           && expression.at().size() == 1){
            if(!lex_expect_optional(Lexer::Punctuator::PCT_semicolon)){
                *expression.at() = Statement::STM_Function;
            }
        } else {
            lex_expect(Lexer::Punctuator::PCT_semicolon);
        }
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

bool Parser::parse_StatementWhile(ParseNode tree)
{
    if(!lex_expect_optional(Lexer::Keyword::KWD_while)){
        return false;
    }
    lex_expect(Lexer::Punctuator::PCT_parenthese_left);
    auto whileStm = tree.append(Statement::STM_While);
    if(!parse_evaluationExpression(whileStm, 0)){
        expected("EvaluationExpression"s, lex());
    }
    lex_expect(Lexer::Punctuator::PCT_parenthese_right);
    if(!parse_Statement(whileStm)){
        expected("Statement"s, lex());
    }
    return true;
}

bool Parser::parse_StatementDoWhile(ParseNode tree)
{
    if(!lex_expect_optional(Lexer::Keyword::KWD_do)){
        return false;
    }
    auto whileStm = tree.append(Statement::STM_DoWhile);
    if(!parse_Statement(whileStm)){
        expected("Statement"s, lex());
    }
    lex_expect(Lexer::Keyword::KWD_while);
    lex_expect(Lexer::Punctuator::PCT_parenthese_left);
    if(!parse_evaluationExpression(whileStm, 0)){
        expected("EvaluationExpression"s, lex());
    }
    lex_expect(Lexer::Punctuator::PCT_parenthese_right);
    lex_expect(Lexer::Punctuator::PCT_semicolon);
    return true;
}

bool Parser::parse_StatementReturn(ParseNode tree)
{
    if(!lex_expect_optional(Lexer::Keyword::KWD_return)){
        return false;
    }
//    if(!tree.find_ascendant(Statement::STM_Function)){
//        expected("Function"s, "Statement(Return)"s);
//    }
    auto statement = tree.append(Statement::STM_Return);
    if(!parse_evaluationExpression(statement, 0)){
        expected("EvaluationExpression"s, lex());
    }
    lex_expect(Lexer::Punctuator::PCT_semicolon);
    return true;
}

bool Parser::parse_varDecl(ParseNode tree)
{
    if(!lex_expect_optional(Lexer::Keyword::KWD_var)){
        return false;
    }
    auto varDeclNode = tree.append(VarDecl{lex_expect_identifier()});
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
    if(parse_GroupingOperation(tree, opr_precedence)
    || parse_JsonObjectOperation(tree, opr_precedence)
    || parse_ArrayObjectOperation(tree, opr_precedence)
    || parse_FunctionOperation(tree, opr_precedence)){
        return true;
    }

    if(parse_MemberAccessOperation(tree, opr_precedence)
    || parse_NewOperation(tree, opr_precedence)){
        return true;
    }

    if(parse_CallOperation(tree, opr_precedence)){
        return true;
    }

    if(parse_postfixUnaryOperation(Lexer::Punctuator::PCT_double_plus,  Operation::OPR_PostfixIncrement, tree, opr_precedence)
    || parse_postfixUnaryOperation(Lexer::Punctuator::PCT_double_minus, Operation::OPR_PostfixDecrement, tree, opr_precedence)){
        return true;
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

    if(parse_binaryLROperation(Lexer::Punctuator::PCT_times,        Operation::OPR_Multiplication, tree, opr_precedence)
    || parse_binaryRLOperation(Lexer::Punctuator::PCT_double_times, Operation::OPR_Exponentiation, tree, opr_precedence)
    || parse_binaryLROperation(Lexer::Punctuator::PCT_divided,      Operation::OPR_Division,       tree, opr_precedence)
    || parse_binaryLROperation(Lexer::Punctuator::PCT_modulo,       Operation::OPR_Remainder,      tree, opr_precedence)){
        return true;
    }

    if(parse_binaryLROperation(Lexer::Punctuator::PCT_plus,  Operation::OPR_Addition,     tree, opr_precedence)
    || parse_binaryLROperation(Lexer::Punctuator::PCT_minus, Operation::OPR_Substraction, tree, opr_precedence)){
        return true;
    }

    if(parse_binaryLROperation(Lexer::Punctuator::PCT_double_lower,   Operation::OPR_BitwiseLeftShift,        tree, opr_precedence)
    || parse_binaryLROperation(Lexer::Punctuator::PCT_double_greater, Operation::OPR_BitwiseRightShift,       tree, opr_precedence)
    || parse_binaryLROperation(Lexer::Punctuator::PCT_triple_greater, Operation::OPR_BitwiseUnsignedRightShift, tree, opr_precedence)){
        return true;
    }

    if(parse_binaryLROperation(Lexer::Punctuator::PCT_lower_than,         Operation::OPR_LessThan,           tree, opr_precedence)
    || parse_binaryLROperation(Lexer::Punctuator::PCT_lower_equal_than,   Operation::OPR_LessThanOrEqual,    tree, opr_precedence)
    || parse_binaryLROperation(Lexer::Punctuator::PCT_greater_than,       Operation::OPR_GreaterThan,        tree, opr_precedence)
    || parse_binaryLROperation(Lexer::Punctuator::PCT_greater_equal_than, Operation::OPR_GreaterThanOrEqual, tree, opr_precedence)
    || parse_binaryLROperation(Lexer::Keyword::KWD_in,                    Operation::OPR_In,                 tree, opr_precedence)
    || parse_binaryLROperation(Lexer::Keyword::KWD_instanceof,            Operation::OPR_InstanceOf,         tree, opr_precedence)){
        return true;
    }

    if(parse_binaryLROperation(Lexer::Punctuator::PCT_double_equal,     Operation::OPR_Equality,         tree, opr_precedence)
    || parse_binaryLROperation(Lexer::Punctuator::PCT_not_equal,        Operation::OPR_Inequality,       tree, opr_precedence)
    || parse_binaryLROperation(Lexer::Punctuator::PCT_triple_equal,     Operation::OPR_StrictEquality,   tree, opr_precedence)
    || parse_binaryLROperation(Lexer::Punctuator::PCT_not_double_equal, Operation::OPR_StrictInequality, tree, opr_precedence)){
        return true;
    }

    if(parse_binaryLROperation(Lexer::Punctuator::PCT_and,         Operation::OPR_BitwiseAND, tree, opr_precedence)
    || parse_binaryLROperation(Lexer::Punctuator::PCT_xor,         Operation::OPR_BitwiseXOR, tree, opr_precedence)
    || parse_binaryLROperation(Lexer::Punctuator::PCT_pipe,        Operation::OPR_BitwiseOR,  tree, opr_precedence)
    || parse_binaryLROperation(Lexer::Punctuator::PCT_double_and,  Operation::OPR_LogicalAND, tree, opr_precedence)
    || parse_binaryLROperation(Lexer::Punctuator::PCT_double_pipe, Operation::OPR_LogicalOR,  tree, opr_precedence)){
        return true;
    }

    if(parse_ConditionalOperation(tree, opr_precedence)){
        return true;
    }

    if(parse_binaryRLOperation(Lexer::Punctuator::PCT_equal,                Operation::OPR_Assignment,                   tree, opr_precedence)
    || parse_binaryRLOperation(Lexer::Punctuator::PCT_plus_equal,           Operation::OPR_AdditionAssignment,           tree, opr_precedence)
    || parse_binaryRLOperation(Lexer::Punctuator::PCT_minus_equal,          Operation::OPR_SubstractAssignment,          tree, opr_precedence)
    || parse_binaryRLOperation(Lexer::Punctuator::PCT_times_equal,          Operation::OPR_MultiplicationAssignment,     tree, opr_precedence)
    || parse_binaryRLOperation(Lexer::Punctuator::PCT_divided_equal,        Operation::OPR_DivisionAssignment,           tree, opr_precedence)
    || parse_binaryRLOperation(Lexer::Punctuator::PCT_modulo_equal,         Operation::OPR_RemainderAssignment,          tree, opr_precedence)
    || parse_binaryRLOperation(Lexer::Punctuator::PCT_double_lower_equal,   Operation::OPR_LeftShiftAssignment,          tree, opr_precedence)
    || parse_binaryRLOperation(Lexer::Punctuator::PCT_double_greater_equal, Operation::OPR_RightShiftAssignment,         tree, opr_precedence)
    || parse_binaryRLOperation(Lexer::Punctuator::PCT_triple_greater_equal, Operation::OPR_UnsignedRightShiftAssignment, tree, opr_precedence)
    || parse_binaryRLOperation(Lexer::Punctuator::PCT_and_equal,            Operation::OPR_BitwiseANDAssignment,         tree, opr_precedence)
    || parse_binaryRLOperation(Lexer::Punctuator::PCT_xor_equal,            Operation::OPR_BitwiseXORAssignment,         tree, opr_precedence)
    || parse_binaryRLOperation(Lexer::Punctuator::PCT_pipe_equal,           Operation::OPR_BitwiseORAssignment,          tree, opr_precedence)){
        return true;
    }

    if(parse_YieldOperation(tree, opr_precedence)){
        return true;
    }

    if(parse_prefixUnaryOperation(Lexer::Punctuator::PCT_spread, Operation::OPR_Spread, tree, opr_precedence)){
        return true;
    }

//    if(parse_binaryLROperation(Lexer::Punctuator::PCT_comma, Operation::OPR_Comma, tree, opr_precedence)){
//        return true;
//    }

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

bool Parser::parse_postfixUnaryOperation(Lexem lxm, Operation opr, ParseNode tree, int opr_precedence)
{
    if(tree.empty()){
        return false;
    }
    if(!lex_expect_optional(lxm)){
        return false;
    }
    previousWrapPrecedence(tree, opr);
    return true;
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

auto Parser::parse_binaryLROperation(Lexem lxm, Operation opr, ParseNode tree, int opr_precedence) -> std::optional<ParseNode>
{
    if(tree.empty()){
        return std::nullopt;
    }
    if(opr_precedence > 0){
        auto treeOpr = std::get_if<Operation>(&*tree);
        if(treeOpr && *treeOpr != Operation::OPR_Grouping && precedence(*treeOpr) >= precedence(opr)){
            return std::nullopt;
        }
    }
    if(!lex_expect_optional(lxm)){
        return std::nullopt;
    }
    auto previous = previousWrapPrecedence(tree, opr);
    if(!parse_evaluationExpression(previous, 0)){
        expected("EvaluationExpression"s, lex());
    }
    return previous;
}

auto Parser::parse_binaryRLOperation(Lexem lxm, Operation opr, ParseNode tree, int opr_precedence) -> std::optional<ParseNode>
{
    if(tree.empty()){
        return std::nullopt;
    }
    if(opr_precedence > 0){
        auto treeOpr = std::get_if<Operation>(&*tree);
        if(treeOpr && *treeOpr != Operation::OPR_Grouping  && precedence(*treeOpr) > precedence(opr)){
            return std::nullopt;
        }
    }
    if(!lex_expect_optional(lxm)){
        return std::nullopt;
    }
    auto previous = previousWrapPrecedence(tree, opr);
    if(!parse_evaluationExpression(previous, 0)){
        expected("EvaluationExpression"s, lex());
    }
    return previous;
}

bool Parser::parse_GroupingOperation(ParseNode tree, int opr_precedence)
{
    if(opr_precedence > 0){
        return false;
    }
    if(!lex_expect_optional(Lexer::Punctuator::PCT_parenthese_left)){
        return false;
    }
    auto operation = tree.append(Operation::OPR_Grouping);
    if(!parse_evaluationExpression(operation, 0)){
        expected("EvaluationExpression"s, lex());
    }
    lex_expect(Lexer::Punctuator::PCT_parenthese_right);
    return true;
}

bool Parser::parse_JsonObjectOperation(ParseNode tree, int opr_precedence)
{
    if(opr_precedence > 0){
        return false;
    }
    if(!lex_expect_optional(Lexer::Punctuator::PCT_brace_left)){
        return false;
    }
    auto operation = tree.append(Operation::OPR_JsonObject);
    if(lex_expect_optional(Lexer::Punctuator::PCT_brace_right)){
        return true;
    }
    do{
        if(lex_expect_optional(Lexer::Punctuator::PCT_bracket_left)){
            auto prop = operation.append(Operation::OPR_Grouping);
            if(!parse_evaluationExpression(prop, 0)){
                expected("EvaluationExpression"s, lex());
            }
            lex_expect(Lexer::Punctuator::PCT_bracket_right);
            lex_expect(Lexer::Punctuator::PCT_colon);
            if(!parse_evaluationExpression(operation, 0)){
                expected("EvaluationExpression"s, lex());
            }
        } else {
            std::string name;
            {
                auto lxm = lex();
                if(auto ident = std::get_if<Lexer::Identifier>(&lxm)){
                    name = *ident;
                } else if(auto kwd = std::get_if<Lexer::Keyword>(&lxm)){
                    name = Lexer::KeywordStr[fys::underlying_cast(*kwd)];
                } else if(auto lit = std::get_if<Lexer::Literal>(&lxm)){
                    name = (*lit).to_string();
                } else {
                    expected("NameIdentifier"s, lxm);
                }
            }
            operation.append(name);
            if(lex_expect_optional(Lexer::Punctuator::PCT_colon)){
                auto prop = operation.append(Operation::OPR_Grouping);
                if(!parse_evaluationExpression(prop, 0)){
                    expected("EvaluationExpression"s, lex());
                }
/*
            } else if(lex_expect_optional(Lexer::Punctuator::PCT_parenthese_left)){
                //parse function
*/
/*
            } else if(name == "get" || name == "set"){
                //parse accessors
*/
            } else {
                operation.append(VarUse{name});
            }
        }
    }while(lex_expect_optional(Lexer::Punctuator::PCT_comma));
    lex_expect(Lexer::Punctuator::PCT_brace_right);
    return true;
}

bool Parser::parse_ArrayObjectOperation(ParseNode tree, int opr_precedence)
{
    if(opr_precedence > 0){
        return false;
    }
    if(!lex_expect_optional(Lexer::Punctuator::PCT_bracket_left)){
        return false;
    }
    if(lex_expect_optional(Lexer::Punctuator::PCT_bracket_right)){
        return true;
    }
    auto operation = tree.append(Operation::OPR_ArrayObject);
    do{
        if(!parse_evaluationExpression(operation, 0)){
            operation.append(var{});
        }
    }while(lex_expect_optional(Lexer::Punctuator::PCT_comma));
    lex_expect(Lexer::Punctuator::PCT_bracket_right);
    return true;
}

bool Parser::parse_FunctionOperation(ParseNode tree, int opr_precedence)
{
    if(opr_precedence > 0){
        return false;
    }
    if(!lex_expect_optional(Lexer::Keyword::KWD_function)){
        return false;
    }
    auto operation = tree.append(Operation::OPR_Function);
    if(auto ident = lex_expect_optional_identifier()){
        operation.append(*ident);
    } else {
        operation.append(var{});
    }
    lex_expect(Lexer::Punctuator::PCT_parenthese_left);
    if(!lex_expect_optional(Lexer::Punctuator::PCT_parenthese_right)){
        do{
            operation.append(lex_expect_identifier());
        }while(lex_expect_optional(Lexer::Punctuator::PCT_comma));
        lex_expect(Lexer::Punctuator::PCT_parenthese_right);
    }
    if(!parse_Statement(operation)){
        expected("FunctionBody"s, lex());
    }
    return true;
}

bool Parser::parse_MemberAccessOperation(ParseNode tree, int opr_precedence)
{
    if(tree.empty()){
        return false;
    }
    if(lex_expect_optional(Lexer::Punctuator::PCT_point)){
        auto previous = tree.last_child();
        previous.wrap(Operation::OPR_MemberAccess);
        previous.append(lex_expect_identifier());
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
    return false;
}

bool Parser::parse_NewOperation(ParseNode tree, int opr_precedence)
{
    if(!lex_expect_optional(Lexer::Keyword::KWD_new)){
        return false;
    }
    auto operation = tree.append(Operation::OPR_New);
    if(!parse_evaluationExpression(operation, 0)){
        expected("EvaluationExpression"s, lex());
    }
    if(!lex_expect_optional(Lexer::Punctuator::PCT_parenthese_left)){
        return true;
    }
    if(parse_evaluationExpression(operation, 0)){
        while(lex_expect_optional(Lexer::Punctuator::PCT_comma)){
            if(!parse_evaluationExpression(operation, 0)){
                expected("EvaluationExpression"s, lex());
            }
        }
    }
    lex_expect(Lexer::Punctuator::PCT_parenthese_right);
    return true;
}

bool Parser::parse_CallOperation(ParseNode tree, int opr_precedence)
{
    if(tree.empty()){
        return false;
    }
    if(opr_precedence > 0){
        auto treeOpr = std::get_if<Operation>(&*tree);
        if(treeOpr && precedence(*treeOpr) > precedence(Operation::OPR_Call)){
            return false;
        }
    }
    if(!lex_expect_optional(Lexer::Punctuator::PCT_parenthese_left)){
        return false;
    }
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

bool Parser::parse_ConditionalOperation(ParseNode tree, int opr_precedence)
{
    if(auto operation = parse_binaryRLOperation(Lexer::Punctuator::PCT_question, Operation::OPR_Conditional, tree, opr_precedence)){
        lex_expect(Lexer::Punctuator::PCT_colon);
        if(!parse_evaluationExpression(*operation, 0)){
            expected("EvaluationExpression"s, lex());
        }
        return true;
    }
    return false;
}

bool Parser::parse_YieldOperation(ParseNode tree, int opr_precedence)
{
    if(opr_precedence > 0){
        return false;
    }
    if(!lex_expect_optional(Lexer::Keyword::KWD_yield)){
        return false;
    }
    auto previous = previousWrapPrecedence(tree, Operation::OPR_Yield);
    parse_evaluationExpression(previous, 0);
    return true;
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
    if(auto ident = lex_expect_optional_identifier()){
        tree.append(VarUse{std::move(*ident)});
        return true;
    }
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

auto Parser::lex_expect_optional_identifier() -> std::optional<Lexer::Identifier>
{
    Lexem lxm = lex();
    if(auto ident = std::get_if<Lexer::Identifier>(&lxm)){
        return *ident;
    }
    lex_putback(std::move(lxm));
    return std::nullopt;
}

auto Parser::lex_expect_identifier() -> Lexer::Identifier
{
    Lexem lxm = lex();
    if(auto ident = std::get_if<Lexer::Identifier>(&lxm)){
        return *ident;
    }
    expected(Lexer::Identifier{}, lxm);
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

