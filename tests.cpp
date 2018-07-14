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
}

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

TEST_CASE("Parser", "[parser]"){
    std::istringstream is;
    std::ostringstream os;
    Lexer lexer({
        [&is]{ return is.peek(); },
        [&is]{ return is.get(); },
        [&is]{ return is.peek() == decltype(is)::traits_type::eof(); }
    });
    Parser parser{lexer};

    SECTION("Variable declaration"){
        is.str("var x;");
        auto tree = parser.parse();

        os << '\n' << tree;
        CHECK(os.str() == R"Parser(
1:Statement(0:TranslationUnit)
>1:Statement(1:Expression)
>>-3:VarDecl(name:x)
)Parser");
    }
    SECTION("Variable declaration and initiatization with literal"){
        is.str("var x = 3;");
        auto tree = parser.parse();

        os << '\n' << tree;
        CHECK(os.str() == R"Parser(
1:Statement(0:TranslationUnit)
>1:Statement(1:Expression)
>>1:VarDecl(name:x)
>>>-4:Literal(3.000000)
)Parser");
    }
    SECTION("Variable declaration and initialization with other variable"){
        is.str("var x = y;");
        auto tree = parser.parse();

        os << '\n' << tree;
        CHECK(os.str() == R"Parser(
1:Statement(0:TranslationUnit)
>1:Statement(1:Expression)
>>1:VarDecl(name:x)
>>>-4:VarUse(name:y)
)Parser");
    }
    SECTION("Multiple statements in translation unit"){
        is.str("var x = 3; var y = x; var z;");
        auto tree = parser.parse();

        os << '\n' << tree;
        CHECK(os.str() == R"Parser(
1:Statement(0:TranslationUnit)
>1:Statement(1:Expression)
>>1:VarDecl(name:x)
>>>-2:Literal(3.000000)
>1:Statement(1:Expression)
>>1:VarDecl(name:y)
>>>-2:VarUse(name:x)
>1:Statement(1:Expression)
>>-3:VarDecl(name:z)
)Parser");
    }
    SECTION("Multiple statements and block in translation unit"){
        is.str("var x = 3; { var y = x; } var z;");
        auto tree = parser.parse();

        os << '\n' << tree;
        CHECK(os.str() == R"Parser(
1:Statement(0:TranslationUnit)
>1:Statement(1:Expression)
>>1:VarDecl(name:x)
>>>-2:Literal(3.000000)
>1:Statement(2:Block)
>>1:Statement(1:Expression)
>>>1:VarDecl(name:y)
>>>>-3:VarUse(name:x)
>1:Statement(1:Expression)
>>-3:VarDecl(name:z)
)Parser");
    }
}
