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
//
//    var v = 2 < 1;
//    std::cout << v << std::endl;
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
