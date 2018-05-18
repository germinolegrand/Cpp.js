#include "Parser.h"

Parser::Parser(Lexer& source):
    m_source(source)
{

}

auto Parser::parse() -> ParseTree
{
    ParseTree root(Statement::STM_Expression);

    ParseTree* current = &root;

    Lexem lxm = m_source.lex();

    if(auto* keyword = std::get_if<Lexer::Keyword>(&lxm); keyword && *keyword == Lexer::Keyword::KWD_var){
        Lexem name = m_source.lex();
        auto* nameIdent = std::get_if<Lexer::Identifier>(&name);
        if(!nameIdent){
            expected(Lexer::Identifier{}, name);
        }
        current->addChild(ParseTree{ParseResult{VarDecl{std::move(*nameIdent)}}});
    }

    return root;
}

void Parser::ParseTree::addChild(ParseTree&& child)
{
    child.parent = this;
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

