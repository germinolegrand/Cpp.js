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

    interpreter.globalEnvironment() = var{{
        {"console", {{
            {"log", var(
                [&os](auto args){
                    std::copy(std::begin(args), std::end(args), std::ostream_iterator<var>(os));
                    os << '\n';
                    return var{};
                })
            }
        }}},
        {"exit", var(
            [](auto args)->var{
                exit(args.size() >= 1 ? static_cast<int>(args[0].to_double()) : 0);
            })
        }
    }};

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
        is.str("var x = (1 + 19)*5/4%7 - 1;");
        auto tree = parser.parse();
        interpreter.feed(tree);

        os << '\n' << interpreter.execute() << '\n';
        CHECK(os.str() == R"Interpreter(
3
)Interpreter");
    }
    SECTION("MemberAccess Arithmetic Assignments"){
        is.str("var x = {a:1}; x.a += 19; x.a *= 5; x.a /= 4; x.a %= 7; x.a -= 1;");
        auto tree = parser.parse();
        interpreter.feed(tree);

        os << '\n' << interpreter.execute() << '\n';
        CHECK(os.str() == R"Interpreter(
3
)Interpreter");
    }
    SECTION("Call operation with arguments"){
        is.str("console.log(2+2, '='+4);");
        auto tree = parser.parse();
        interpreter.feed(tree);

        os << '\n' << interpreter.execute() << '\n';
        CHECK(os.str() == R"Interpreter(
4=4
undefined
)Interpreter");
    }
    SECTION("Call operation with undefined arguments"){
        is.str("console.log(x);");
        auto tree = parser.parse();
        interpreter.feed(tree);

        os << '\n' << interpreter.execute() << '\n';
        CHECK(os.str() == R"Interpreter(
undefined
undefined
)Interpreter");
    }
    SECTION("Compare operations"){
        is.str("console.log(1 < 1, 1 < 2, 2 > 1, 2 > 3, 3 <= 2, 3 <= 3, 3 <= '4', 5 >= 3, 5 >= 6, '5' >= '5');");
        auto tree = parser.parse();
        interpreter.feed(tree);

        os << '\n' << interpreter.execute() << '\n';
        CHECK(os.str() == R"Interpreter(
falsetruetruefalsefalsetruetruetruefalsetrue
undefined
)Interpreter");
    }
}
