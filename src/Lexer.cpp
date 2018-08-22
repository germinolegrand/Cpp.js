#include "Lexer.h"


Lexer::Lexer(Source src): m_source(std::move(src))
{
    //ctor
}

auto Lexer::lex() -> Lexem
{
    while(!std::isgraph(m_source.peek()) && !eof()){
        m_source.consume();
    }

    if(eof()){
        return Symbol::SBL_EOF;
    }

    m_current_unit.clear();
    m_canBeKeyword = true;
    m_canBePunctuator = true;
    m_canBeIdentifier = true;
    m_canBeLiteral = true;
    m_canBeLiteralFloat = true;

    while(m_canBeKeyword || m_canBePunctuator || m_canBeIdentifier || m_canBeLiteral){
        std::optional<Lexem> res;

        if((m_canBeKeyword && (res = analyseKeyword()))
        || (m_canBePunctuator && (res = analysePunctuator()))
        || (m_canBeIdentifier && (res = analyseIdentifier()))
        || (m_canBeLiteral && (res = analyseLiteral()))){
            return std::move(*res);
        }

        char current_char = m_source.consume();
        m_current_unit += current_char;
    }

    throw std::invalid_argument("Impossible to lex current unit (" + m_current_unit + ")");
}

bool Lexer::eof()
{
    return m_source.eof();
}

auto Lexer::analyseKeyword() -> std::optional<Lexem>
{
    auto ch = m_source.peek();
    if(std::isalpha(ch)){
        return std::nullopt;
    }
    if(auto it = std::find(std::begin(KeywordStr), std::end(KeywordStr), m_current_unit);
       it != std::end(KeywordStr)){
        return std::make_optional(Keyword{static_cast<std::underlying_type_t<Keyword>>
                                  (it - std::begin(KeywordStr))});
    }
    m_canBeKeyword = false;
    return std::nullopt;
}

auto Lexer::analysePunctuator() -> std::optional<Lexem>
{
    int exactMatch = -1;
    int count = 0;
    for(auto& punc : PunctuatorStr){
        if(punc == m_current_unit){
            exactMatch = static_cast<int>(&punc - std::begin(PunctuatorStr));
            ++count;
        } else if(punc.size() > m_current_unit.size()
                  && punc.compare(0, m_current_unit.size(), m_current_unit) == 0
                  && punc[m_current_unit.size()] == m_source.peek()){
            ++count;
        }
    }
    if(count == 0){
        m_canBePunctuator = false;
        return std::nullopt;
    }
    if(count == 1 && exactMatch != -1){
        return std::make_optional(Punctuator{exactMatch});
    }

    return std::nullopt;
}

auto Lexer::analyseIdentifier() -> std::optional<Lexem>
{
    auto ch = m_source.peek();
    if(m_current_unit.empty()){
        if(!std::isalpha(ch) && ch != '$' && ch != '_'){
            m_canBeIdentifier = false;
        }
        return std::nullopt;
    }
    if(!std::isalnum(ch) && ch != '$' && ch != '_'){
        return std::make_optional(m_current_unit);
    }
    return std::nullopt;
}

auto Lexer::analyseLiteral() -> std::optional<Lexem>
{
    auto ch = m_source.peek();

    if(m_current_unit.empty()){
        if(std::isdigit(ch) || ch == '.'){
            return analyseLiteralFloat();
        }
        if(ch == '"' || ch == '\''){
            return analyseLiteralString();
        }
        m_canBeLiteral = false;
        return std::nullopt;
    }
    std::optional<Lexem> res;
    if(m_canBeLiteralFloat && (res = analyseLiteralFloat())){
        return res;
    }
    return std::nullopt;
}

auto Lexer::analyseLiteralFloat() -> std::optional<Lexem>
{
    auto ch = m_source.peek();
    if(m_current_unit.empty()){
        if(!std::isdigit(ch) && ch != '.'){
            m_canBeLiteralFloat = false;
            return std::nullopt;
        }
    }
    bool rejectDot = false;
    if(ch == '.'){
        rejectDot = m_current_unit.find('.') != std::string::npos;
    }
    if((!std::isdigit(ch) && ch != '.') || rejectDot){
        return std::make_optional(var(double(std::stold(m_current_unit))));
    }
    return std::nullopt;
}

auto Lexer::analyseLiteralString() -> std::optional<Lexem>
{
    std::string ret;
    auto quote_ch = m_source.consume();
    bool escaped = false;
    char ch = 0;
    bool endOfFile = false;
    while(!(endOfFile = eof()) && ((ch = m_source.peek()) != quote_ch || escaped)){
        if(escaped){
            switch(ch)
            {
            case '\\':
            case '\'':
            case '\"':
                ret += m_source.consume();
                break;
            case 'n':
                ret += '\n';
                m_source.consume();
                break;
            case 't':
                ret += '\t';
                m_source.consume();
                break;
            case 'v':
                ret += '\v';
                m_source.consume();
                break;
            default:
                throw std::invalid_argument("Unknown escape sequence while lexing Literal string");
            }
            escaped = false;
        } else if(ch == '\\'){
            escaped = true;
            m_source.consume();
        } else {
            ret += m_source.consume();
        }
    }
    if(endOfFile){
        throw std::invalid_argument("Unexpected EOF while lexing Literal string");
    }
    m_source.consume();
    return std::make_optional(var(std::move(ret)));
}

