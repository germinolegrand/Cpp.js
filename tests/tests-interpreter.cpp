#include <catch2/catch.hpp>

#include <iostream>

#include "Interpreter.h"

TEST_CASE("Interpreter", "[interpreter]"){
    std::istringstream is;
    std::ostringstream os;
    Lexer lexer({
        [&is]{ return is.peek(); },
        [&is]{ return is.get(); },
        [&is]{ return is.peek() == decltype(is)::traits_type::eof(); }
    });
    Parser parser{lexer};
    Interpreter interpreter;

    SECTION("Variable declaration"){
        is.str("var x = 3;");
        auto tree = parser.parse();
        interpreter.feed(tree);

        os << '\n' << interpreter.execute() << '\n';
        CHECK(os.str() == R"Interpreter(
3
)Interpreter");
    }
    SECTION("Variable declaration"){
        is.str("var x = 3; var y; var z = x;");
        auto tree = parser.parse();
        interpreter.feed(tree);

        os << '\n' << interpreter.execute() << '\n';
        CHECK(os.str() == R"Interpreter(
3
)Interpreter");
    }
    SECTION("JsonObject simple"){
        is.str("var x = {}; var y = {'abc':'nooo', 34:42, x};");
        auto tree = parser.parse();
        interpreter.feed(tree);

        os << '\n' << interpreter.execute() << '\n';
        CHECK(os.str() == R"Interpreter(
{"x":{},"abc":"nooo","34":42}
)Interpreter");
    }
    SECTION("MemberAccess Assignment"){
        is.str("var x = 'foo'; x = {a:4}; x.b = x.a; x.a = 5; x;");
        auto tree = parser.parse();
        interpreter.feed(tree);

        os << '\n' << interpreter.execute() << '\n';
        CHECK(os.str() == R"Interpreter(
{"b":4,"a":5}
)Interpreter");
    }
    SECTION("Addition"){
        is.str("var x = {a: 1 + 1, b: 'coucou' + false, c: 2 + true, d: true + false};");
        auto tree = parser.parse();
        interpreter.feed(tree);

        os << '\n' << interpreter.execute() << '\n';
        CHECK(os.str() == R"Interpreter(
{"c":3,"a":2,"d":1,"b":"coucoufalse"}
)Interpreter");
    }
    SECTION("Basic arithmetic operations"){
        is.str("var x = (1 + 19)*5/4%7;");
        auto tree = parser.parse();
        interpreter.feed(tree);

        os << '\n' << interpreter.execute() << '\n';
        CHECK(os.str() == R"Interpreter(
4
)Interpreter");
    }
}
