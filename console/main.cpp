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

    interpreter.globalEnvironment() = var{{
        {"console", {{
            {"log", var(
                [](auto args){
                    std::copy(std::begin(args), std::end(args), std::ostream_iterator<var>(std::cout));
                    std::cout << '\n';
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
