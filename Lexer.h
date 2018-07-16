#pragma once

#include "var.h"

#include <optional>

#include <fys/utility/underlying_cast.h>

using namespace std::literals;

class Lexer
{
public:
    struct Source
    {
        std::function<char()> peek;
        std::function<char()> consume;
        std::function<bool()> eof;
    };

    enum class Keyword
    {
        KWD_var,
        KWD_new,
        KWD_function,
        KWD_return,
        KWD_if,
        KWD_else,
        KWD_for,
        KWD_while,
        KWD_typeof,
        KWD_void,
        KWD_delete,
        KWD_this,
        KWD_prototype,
        KWD_undefined,
        KWD_true,
        KWD_false,
        KWD_null,
    };

    static constexpr std::string_view KeywordStr[] =
    {
        "var"sv,
        "new"sv,
        "function"sv,
        "return"sv,
        "if"sv,
        "else"sv,
        "for"sv,
        "while"sv,
        "typeof"sv,
        "void"sv,
        "delete"sv,
        "this"sv,
        "prototype"sv,
        "undefined"sv,
        "true"sv,
        "false"sv,
        "null"sv,
    };

    enum class Punctuator
    {
        PCT_point           ,//    = 0x1200,
        PCT_comma           ,//        = 0x0001,
        PCT_colon           ,//        = 0x0002,
        PCT_semicolon       ,//        = 0x0003,
        PCT_parenthese_left ,//    = 0x1004,
        PCT_parenthese_right,//        = 0x0005,
        PCT_brace_left      ,//    = 0x1306,
        PCT_brace_right     ,//        = 0x0007,
        PCT_bracket_left    ,//    = 0x1208,
        PCT_bracket_right   ,//        = 0x0009,
        PCT_lower_than      ,//    = 0x0b0a,
        PCT_greater_than    ,//    = 0x0b0b,
        PCT_equal           ,//    = 0x030c,
        PCT_double_equal    ,//    = 0x0a0d,
        PCT_triple_equal    ,//    = 0x0a0e,
        PCT_plus            ,//        = 0x0010,
        PCT_minus           ,//        = 0x0011,
        PCT_times           ,//        = 0x0012,
        PCT_divided         ,//        = 0x0013,
        PCT_modulo          ,//        = 0x0014,
        PCT_spread          ,
        PCT_lower_equal_than,
        PCT_greater_equal_than,
        PCT_not_equal,
        PCT_not_double_equal,
        PCT_double_plus,
        PCT_double_minus,
        PCT_double_lower,
        PCT_double_greater,
        PCT_triple_greater,
        PCT_and,
        PCT_pipe,
        PCT_xor,
        PCT_not,
        PCT_tilde,
        PCT_double_and,
        PCT_double_pipe,
        PCT_question,
        PCT_plus_equal,
        PCT_minus_equal,
        PCT_times_equal,
        PCT_divided_equal,
        PCT_modulo_equal,
        PCT_double_lower_equal,
        PCT_double_greater_equal,
        PCT_triple_greater_equal,
        PCT_and_equal,
        PCT_pipe_equal,
        PCT_xor_equal,
        PCT_equal_greater,
    };

    static constexpr std::string_view PunctuatorStr[] =
    {
        "."sv,
        ","sv,
        ":"sv,
        ";"sv,
        "("sv,
        ")"sv,
        "{"sv,
        "}"sv,
        "["sv,
        "]"sv,
        "<"sv,
        ">"sv,
        "="sv,
        "=="sv,
        "==="sv,
        "+"sv,
        "-"sv,
        "*"sv,
        "/"sv,
        "%"sv,
        "..."sv,
        "<="sv,
        ">="sv,
        "!="sv,
        "!=="sv,
        "++"sv,
        "--"sv,
        "<<"sv,
        ">>"sv,
        ">>>"sv,
        "&"sv,
        "|"sv,
        "^"sv,
        "!"sv,
        "~"sv,
        "&&"sv,
        "||"sv,
        "?"sv,
        "+="sv,
        "-="sv,
        "*="sv,
        "/="sv,
        "%="sv,
        "<<="sv,
        ">>="sv,
        ">>>="sv,
        "&="sv,
        "|="sv,
        "^="sv,
        "=>"sv,
    };

    using Identifier = std::string;

    using Literal = var;

    using Lexem = std::variant
    <
        Keyword,
        Punctuator,
        Identifier,
        Literal
    >;
public:
    Lexer(Source src);

    Lexem lex();

    bool eof();

private:
    Source m_source;

    bool m_canBeKeyword = true;
    bool m_canBePunctuator = true;
    bool m_canBeIdentifier = true;
    bool m_canBeLiteral = true;
    bool m_canBeLiteralFloat = true;

    std::string m_current_unit;

    std::optional<Lexem> analyseKeyword();
    std::optional<Lexem> analysePunctuator();
    std::optional<Lexem> analyseIdentifier();
    std::optional<Lexem> analyseLiteral();
    std::optional<Lexem> analyseLiteralFloat();
    std::optional<Lexem> analyseLiteralString();
};

inline std::ostream& operator<<(std::ostream& os, Lexer::Lexem const& lxm)
{
    if(auto kw = std::get_if<Lexer::Keyword>(&lxm)){
        return os << "Keyword(" << Lexer::KeywordStr[fys::underlying_cast(*kw)] << ")";
    } else if(auto pct = std::get_if<Lexer::Punctuator>(&lxm)){
        return os << "Punctuator(" << Lexer::PunctuatorStr[fys::underlying_cast(*pct)&0xff] << ")";
    } else if(auto idt = std::get_if<Lexer::Identifier>(&lxm)){
        return os << "Identifier(" << *idt << ")";
    } else if(auto lit = std::get_if<Lexer::Literal>(&lxm)){
        return os << "Literal(" << *lit << ")";
    }
    return os;
}
