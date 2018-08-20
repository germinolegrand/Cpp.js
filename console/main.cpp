#include "Interpreter.h"

#include <iostream>

int main()
{
    Lexer lexer({
        []{ return std::cin.peek(); },
        []{ return std::cin.get(); },
        []{ return std::cin.peek() == decltype(std::cin)::traits_type::eof(); }
    });
    Parser parser{lexer};
    Interpreter interpreter;

    while(true){
        std::cout << "Cpp.js> " << std::flush;
        try{
            interpreter.feed(parser.parse());
        }catch(std::invalid_argument& e){
            std::cout << '\n' << "ParseError: " << e.what() << '\n';
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max());
            continue;
        }
        try{
            std::cout << '\n' << interpreter.execute() << '\n';
        }catch(Interpreter::unimplemented_error& e){
            std::cout << '\n' << "Error: " << e.what() << " is not implemented yet." << '\n';
        }
        std::cin.clear();
    }
}
