#include "Parser.h"

Parser::Parser(Lexer& source):
    m_source(source)
{

}

auto Parser::parse() -> ParseTree
{
    ParseTree root(Statement::STM_Expression);

    parse_StatementExpression(root);

    return root;
}

bool Parser::parse_StatementExpression(ParseTree& tree)
{
    ParseTree expression(Statement::STM_Expression);
    if(parse_varDecl(expression) || parse_evaluationExpression(expression, 0)){
        Lexem separator = lex();
        if(auto* punc = std::get_if<Lexer::Punctuator>(&separator); punc && *punc == Lexer::Punctuator::PCT_semicolon){
            tree.addChild(std::make_unique<ParseTree>(std::move(expression)));
            return true;
        } else {
            expected(Lexer::Punctuator::PCT_semicolon, separator);
        }
    }

    return false;
}

bool Parser::parse_varDecl(ParseTree& tree)
{
    Lexem lxm = lex();

    if(auto* keyword = std::get_if<Lexer::Keyword>(&lxm); keyword && *keyword == Lexer::Keyword::KWD_var){
        Lexem name = lex();
        auto* nameIdent = std::get_if<Lexer::Identifier>(&name);
        if(!nameIdent){
            expected(Lexer::Identifier{}, name);
        }
        tree.addChild(std::make_unique<ParseTree>(ParseResult{VarDecl{std::move(*nameIdent)}}));

        Lexem initialisation = lex();
        if(auto* eq = std::get_if<Lexer::Punctuator>(&initialisation); eq && *eq == Lexer::Punctuator::PCT_equal){
            parse_evaluationExpression(*tree.children.back(), 0);
        } else {
            lex_putback(std::move(initialisation));
        }

        return true;
    }

    return false;
}

bool Parser::parse_evaluationExpression(ParseTree& tree, int opr_precedence)
{
    bool ret = false;
    if(parse_Literal(tree) || parse_varUse(tree)){
        ret = true;
    }
    while(parse_operator(tree, opr_precedence)){
        ret = true;
    }
    return ret;
}

bool Parser::parse_operator(ParseTree& tree, int opr_precedence)
{
    return false;
}

bool Parser::parse_Literal(ParseTree& tree)
{
    Lexem lxm = lex();
    if(auto* lit = std::get_if<Lexer::Literal>(&lxm); lit){
        tree.addChild(std::make_unique<ParseTree>(ParseResult{std::move(*lit)}));
        return true;
    }
    lex_putback(std::move(lxm));
    return false;
}

bool Parser::parse_varUse(ParseTree& tree)
{
    Lexem name = lex();
    if(auto* nameIdent = std::get_if<Lexer::Identifier>(&name); nameIdent){
        tree.addChild(std::make_unique<ParseTree>(ParseResult{VarUse{std::move(*nameIdent)}}));
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

void Parser::ParseTree::addChild(std::unique_ptr<ParseTree>&& child)
{
    child->parent = this;
    children.push_back(std::move(child));
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

