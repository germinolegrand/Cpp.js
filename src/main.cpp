#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

/*
#include <iostream>

#include "var.h"
#include "Lexer.h"
#include "Parser.h"

int main()
{
    var a{{{"d", "e"}}};
    var const& ref = a;

    a["b"] = 34.;
    std::cout << a << std::endl;

    std::cout << ref["b"] << std::endl;

    var v = 2 < 1;
    std::cout << v << std::endl;

    Lexer lexer({
        []{ return std::cin.peek(); },
        []{ return std::cin.get(); }
    });

    Parser parser(lexer);

    auto tree = parser.parse();

    std::cout << tree << std::endl;

//    for(int i = 0; i < 10; ++i){
//        auto lexem = lexer.lex();
//        std::cout << lexem << std::endl;
//    }
}

*/
