#include <libs/catch.hpp>

#include <iostream>

#include "var.h"
#include "Lexer.h"
#include "Parser.h"

TEST_CASE("Var", "[var]"){
    std::ostringstream os;

    var a{{{"d", "e"}}};
    var const& ref = a;
    var copie = a;

    a["b"] = 34.;
    os << '\n' << a << '\n';

    CHECK(os.str() == R"(
{"b":34.000000,"d":"e"}
)");
    CHECK(a["b"] == 34);
    CHECK(ref["b"] == 34);
    CHECK(copie["b"] == a["b"]);

    os.str("");

    var v = 2 < 1;
    os << v;
    CHECK(os.str() == "false");

//
//    Lexer lexer({
//        []{ return std::cin.peek(); },
//        []{ return std::cin.get(); }
//    });
//
//    Parser parser(lexer);
//
//    auto tree = parser.parse();
//
//    std::cout << tree << std::endl;
}

TEST_CASE("Lexer", "[lexer]"){
    std::istringstream is{"var x = 3;"};
    Lexer lexer({
        [&is]{ return is.peek(); },
        [&is]{ return is.get(); }
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

TEST_CASE("Parser", "[parser]"){
    std::istringstream is{"var x = 3;"};
    Lexer lexer({
        [&is]{ return is.peek(); },
        [&is]{ return is.get(); }
    });
    Parser parser{lexer};
    auto tree = parser.parse();

    std::ostringstream os;
    os << '\n' << tree;
    CHECK(os.str() == R"Parser(
1:Statement(0)
`1:Statement(0)
``1:VarDecl(name:x)
```-4:Literal(3.000000)
)Parser");
}
