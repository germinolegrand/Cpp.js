#include "Interpreter.h"

#include <iostream>

int main(int argc, char *argv[])
{
    Lexer lexer({
        []{ return std::cin.peek(); },
        []{ return std::cin.get(); },
        []{ return std::cin.peek() == decltype(std::cin)::traits_type::eof(); }
    });
    Parser parser{lexer};
    Interpreter interpreter;

    while(true){
        std::cin.clear();
        std::cout << "Cpp.js> " << std::flush;
        try{
            interpreter.feed(parser.parse());
        }catch(std::invalid_argument& e){
            std::cout << '\n' << "ParseError:" << e.what() << '\n';
            continue;
        }
        try{
            std::cout << '\n' << interpreter.execute() << '\n';
        }catch(unavailable_operation& e){
            std::cout << '\n' << "ExecutionError: unavailable operation" << '\n';
        }
    }
}
