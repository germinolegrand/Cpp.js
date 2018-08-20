#include <catch2/catch.hpp>

#include <iostream>

#include "Lexer.h"

TEST_CASE("Lexer", "[lexer]"){
    std::istringstream is{"var x = 3;"};
    Lexer lexer({
        [&is]{ return is.peek(); },
        [&is]{ return is.get(); },
        [&is]{ return is.peek() == decltype(is)::traits_type::eof(); }
    });
    std::ostringstream os;
    os << '\n' << lexer.lex()
       << '\n' << lexer.lex()
       << '\n' << lexer.lex()
       << '\n' << lexer.lex()
       << '\n' << lexer.lex()
       << '\n';
    CHECK(os.str() == R"Lexer(
Keyword(var)
Identifier(x)
Punctuator(=)
Literal(3.000000)
Punctuator(;)
)Lexer");
}
