#include <libs/catch.hpp>

#include <iostream>

#include "var.h"
#include "Lexer.h"
#include "Parser.h"
#include "Interpreter.h"

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
    SECTION("If statement"){
        is.str("var x = 3; if(true){ var y = x; } var z;");
        auto tree = parser.parse();

        os << '\n' << tree;
        CHECK(os.str() == R"Parser(
1:Statement(0:TranslationUnit)
>1:Statement(1:Expression)
>>1:VarDecl(name:x)
>>>-2:Literal(3.000000)
>1:Statement(3:If)
>>0:Literal(true)
>>1:Statement(2:Block)
>>>1:Statement(1:Expression)
>>>>1:VarDecl(name:y)
>>>>>-4:VarUse(name:x)
>1:Statement(1:Expression)
>>-3:VarDecl(name:z)
)Parser");
    }
    SECTION("If-Else statement"){
        is.str("if(true){ var y = x; } else { var x = 3; } var z;");
        auto tree = parser.parse();

        os << '\n' << tree;
        CHECK(os.str() == R"Parser(
1:Statement(0:TranslationUnit)
>1:Statement(3:If)
>>0:Literal(true)
>>1:Statement(2:Block)
>>>1:Statement(1:Expression)
>>>>1:VarDecl(name:y)
>>>>>-3:VarUse(name:x)
>>1:Statement(2:Block)
>>>1:Statement(1:Expression)
>>>>1:VarDecl(name:x)
>>>>>-4:Literal(3.000000)
>1:Statement(1:Expression)
>>-3:VarDecl(name:z)
)Parser");
    }
    SECTION("Member access point operation"){
        is.str("var x = y.z;");
        auto tree = parser.parse();

        os << '\n' << tree;
        CHECK(os.str() == R"Parser(
1:Statement(0:TranslationUnit)
>1:Statement(1:Expression)
>>1:VarDecl(name:x)
>>>1:Operation(1200:MemberAccess)
>>>>0:VarUse(name:y)
>>>>-5:Literal(z)
)Parser");
    }
    SECTION("Member access bracket operation"){
        is.str("var x = y[z];");
        auto tree = parser.parse();

        os << '\n' << tree;
        CHECK(os.str() == R"Parser(
1:Statement(0:TranslationUnit)
>1:Statement(1:Expression)
>>1:VarDecl(name:x)
>>>1:Operation(1200:MemberAccess)
>>>>0:VarUse(name:y)
>>>>-5:VarUse(name:z)
)Parser");
    }
    SECTION("Member access multiple point operation"){
        is.str("var x = a.b.c;");
        auto tree = parser.parse();

        os << '\n' << tree;
        CHECK(os.str() == R"Parser(
1:Statement(0:TranslationUnit)
>1:Statement(1:Expression)
>>1:VarDecl(name:x)
>>>1:Operation(1200:MemberAccess)
>>>>1:Operation(1200:MemberAccess)
>>>>>0:VarUse(name:a)
>>>>>-1:Literal(b)
>>>>-5:Literal(c)
)Parser");
    }
    SECTION("Call operation"){
        is.str("var x = a.b().c();");
        auto tree = parser.parse();

        os << '\n' << tree;
        CHECK(os.str() == R"Parser(
1:Statement(0:TranslationUnit)
>1:Statement(1:Expression)
>>1:VarDecl(name:x)
>>>1:Operation(1100:Call)
>>>>1:Operation(1200:MemberAccess)
>>>>>1:Operation(1100:Call)
>>>>>>1:Operation(1200:MemberAccess)
>>>>>>>0:VarUse(name:a)
>>>>>>>-2:Literal(b)
>>>>>-6:Literal(c)
)Parser");
    }
    SECTION("Postfix-prefix increment operation"){
        is.str("var x = --!y++;");
        auto tree = parser.parse();

        os << '\n' << tree;
        CHECK(os.str() == R"Parser(
1:Statement(0:TranslationUnit)
>1:Statement(1:Expression)
>>1:VarDecl(name:x)
>>>1:Operation(f01:PrefixDecrement)
>>>>1:Operation(f02:LogicalNot)
>>>>>1:Operation(1000:PostfixIncrement)
>>>>>>-7:VarUse(name:y)
)Parser");
    }
    SECTION("Operations precedences"){
        is.str("var x = --!y.z()++;");
        auto tree = parser.parse();

        os << '\n' << tree;
        CHECK(os.str() == R"Parser(
1:Statement(0:TranslationUnit)
>1:Statement(1:Expression)
>>1:VarDecl(name:x)
>>>1:Operation(f01:PrefixDecrement)
>>>>1:Operation(f02:LogicalNot)
>>>>>1:Operation(1000:PostfixIncrement)
>>>>>>1:Operation(1100:Call)
>>>>>>>1:Operation(1200:MemberAccess)
>>>>>>>>0:VarUse(name:y)
>>>>>>>>-9:Literal(z)
)Parser");
    }
    SECTION("Addition/Multiplication precedences"){
        is.str("var x = a + b * c % d - e / f;");
        auto tree = parser.parse();

        os << '\n' << tree;
        CHECK(os.str() == R"Parser(
1:Statement(0:TranslationUnit)
>1:Statement(1:Expression)
>>1:VarDecl(name:x)
>>>1:Operation(d01:Substraction)
>>>>1:Operation(d00:Addition)
>>>>>0:VarUse(name:a)
>>>>>1:Operation(e03:Remainder)
>>>>>>1:Operation(e00:Multiplication)
>>>>>>>0:VarUse(name:b)
>>>>>>>-1:VarUse(name:c)
>>>>>>-2:VarUse(name:d)
>>>>1:Operation(e02:Division)
>>>>>0:VarUse(name:e)
>>>>>-6:VarUse(name:f)
)Parser");
    }
    SECTION("Multiple operations precedences"){
        is.str("var x = a + b++ * !+c++;");
        auto tree = parser.parse();

        os << '\n' << tree;
        CHECK(os.str() == R"Parser(
1:Statement(0:TranslationUnit)
>1:Statement(1:Expression)
>>1:VarDecl(name:x)
>>>1:Operation(d00:Addition)
>>>>0:VarUse(name:a)
>>>>1:Operation(e00:Multiplication)
>>>>>1:Operation(1000:PostfixIncrement)
>>>>>>-1:VarUse(name:b)
>>>>>1:Operation(f02:LogicalNot)
>>>>>>1:Operation(f04:UnaryPlus)
>>>>>>>1:Operation(1000:PostfixIncrement)
>>>>>>>>-9:VarUse(name:c)
)Parser");
    }
    SECTION("Exponentiation associativity"){
        is.str("a ** 2 ** 3;");
        auto tree = parser.parse();

        os << '\n' << tree;
        CHECK(os.str() == R"Parser(
1:Statement(0:TranslationUnit)
>1:Statement(1:Expression)
>>1:Operation(e01:Exponentiation)
>>>0:VarUse(name:a)
>>>1:Operation(e01:Exponentiation)
>>>>0:Literal(2.000000)
>>>>-5:Literal(3.000000)
)Parser");
    }
    SECTION("RL simple operations associativity"){
        is.str("a = b = c += d;");
        auto tree = parser.parse();

        os << '\n' << tree;
        CHECK(os.str() == R"Parser(
1:Statement(0:TranslationUnit)
>1:Statement(1:Expression)
>>1:Operation(300:Assignment)
>>>0:VarUse(name:a)
>>>1:Operation(300:Assignment)
>>>>0:VarUse(name:b)
>>>>1:Operation(301:AdditionAssignment)
>>>>>0:VarUse(name:c)
>>>>>-6:VarUse(name:d)
)Parser");
    }
    SECTION("RL complex operations associativity"){
        is.str("x = a *= !b - c = !d;");
        auto tree = parser.parse();

        os << '\n' << tree;
        CHECK(os.str() == R"Parser(
1:Statement(0:TranslationUnit)
>1:Statement(1:Expression)
>>1:Operation(300:Assignment)
>>>0:VarUse(name:x)
>>>1:Operation(303:MultiplicationAssignment)
>>>>0:VarUse(name:a)
>>>>1:Operation(300:Assignment)
>>>>>1:Operation(d01:Substraction)
>>>>>>1:Operation(f02:LogicalNot)
>>>>>>>-1:VarUse(name:b)
>>>>>>-1:VarUse(name:c)
>>>>>1:Operation(f02:LogicalNot)
>>>>>>-7:VarUse(name:d)
)Parser");
    }
    SECTION("Conditionnal operation associativity"){
        is.str("x = a >= 3 ? 55 ^ 3 > '1000' ? e : f : b > 42 ? c : d;");
        auto tree = parser.parse();

        os << '\n' << tree;
        CHECK(os.str() == R"Parser(
1:Statement(0:TranslationUnit)
>1:Statement(1:Expression)
>>1:Operation(300:Assignment)
>>>0:VarUse(name:x)
>>>1:Operation(400:Conditional)
>>>>1:Operation(b03:GreaterThanOrEqual)
>>>>>0:VarUse(name:a)
>>>>>-1:Literal(3.000000)
>>>>1:Operation(400:Conditional)
>>>>>1:Operation(800:BitwiseXOR)
>>>>>>0:Literal(55.000000)
>>>>>>1:Operation(b02:GreaterThan)
>>>>>>>0:Literal(3.000000)
>>>>>>>-2:Literal(1000)
>>>>>0:VarUse(name:e)
>>>>>-1:VarUse(name:f)
>>>>1:Operation(400:Conditional)
>>>>>1:Operation(b02:GreaterThan)
>>>>>>0:VarUse(name:b)
>>>>>>-1:Literal(42.000000)
>>>>>0:VarUse(name:c)
>>>>>-6:VarUse(name:d)
)Parser");
    }
    SECTION("New operation"){
        is.str("var x = new carrot; var y = new banana(); var z = new apple(12, 34);");
        auto tree = parser.parse();

        os << '\n' << tree;
        CHECK(os.str() == R"Parser(
1:Statement(0:TranslationUnit)
>1:Statement(1:Expression)
>>1:VarDecl(name:x)
>>>1:Operation(1201:New)
>>>>-3:VarUse(name:carrot)
>1:Statement(1:Expression)
>>1:VarDecl(name:y)
>>>1:Operation(1201:New)
>>>>-3:VarUse(name:banana)
>1:Statement(1:Expression)
>>1:VarDecl(name:z)
>>>1:Operation(1201:New)
>>>>0:VarUse(name:apple)
>>>>0:Literal(12.000000)
>>>>-5:Literal(34.000000)
)Parser");
    }
    SECTION("ArrayObject"){
        is.str("var x = []; var y = [1]; var z = ['abc', 34, crap]; var w = [a, , , c, d, ];");
        auto tree = parser.parse();

        os << '\n' << tree;
        CHECK(os.str() == R"Parser(
1:Statement(0:TranslationUnit)
>1:Statement(1:Expression)
>>-1:VarDecl(name:x)
>1:Statement(1:Expression)
>>1:VarDecl(name:y)
>>>1:Operation(1302:ArrayObject)
>>>>-3:Literal(1.000000)
>1:Statement(1:Expression)
>>1:VarDecl(name:z)
>>>1:Operation(1302:ArrayObject)
>>>>0:Literal(abc)
>>>>0:Literal(34.000000)
>>>>-3:VarUse(name:crap)
>1:Statement(1:Expression)
>>1:VarDecl(name:w)
>>>1:Operation(1302:ArrayObject)
>>>>0:VarUse(name:a)
>>>>0:Literal(undefined)
>>>>0:Literal(undefined)
>>>>0:VarUse(name:c)
>>>>0:VarUse(name:d)
>>>>-5:Literal(undefined)
)Parser");
    }
    SECTION("JsonObject"){
        is.str("var x = {}; var y = {banana:33+34, [f + g]:h}; var z = {'abc':'nooo', 34:42, crap};");
        auto tree = parser.parse();

        os << '\n' << tree;
        CHECK(os.str() == R"Parser(
1:Statement(0:TranslationUnit)
>1:Statement(1:Expression)
>>1:VarDecl(name:x)
>>>-2:Operation(1301:JsonObject)
>1:Statement(1:Expression)
>>1:VarDecl(name:y)
>>>1:Operation(1301:JsonObject)
>>>>0:Literal(banana)
>>>>1:Operation(1300:Grouping)
>>>>>1:Operation(d00:Addition)
>>>>>>0:Literal(33.000000)
>>>>>>-2:Literal(34.000000)
>>>>1:Operation(1300:Grouping)
>>>>>1:Operation(d00:Addition)
>>>>>>0:VarUse(name:f)
>>>>>>-2:VarUse(name:g)
>>>>-3:VarUse(name:h)
>1:Statement(1:Expression)
>>1:VarDecl(name:z)
>>>1:Operation(1301:JsonObject)
>>>>0:Literal(abc)
>>>>1:Operation(1300:Grouping)
>>>>>-1:Literal(nooo)
>>>>0:Literal(34.000000)
>>>>1:Operation(1300:Grouping)
>>>>>-1:Literal(42.000000)
>>>>0:Literal(crap)
>>>>-5:VarUse(name:crap)
)Parser");
    }
    SECTION("JsonObject recursive"){
        is.str("var x = {y:{a:33, b:34}, z:{c:35, d:36}};");
        auto tree = parser.parse();

        os << '\n' << tree;
        CHECK(os.str() == R"Parser(
1:Statement(0:TranslationUnit)
>1:Statement(1:Expression)
>>1:VarDecl(name:x)
>>>1:Operation(1301:JsonObject)
>>>>0:Literal(y)
>>>>1:Operation(1300:Grouping)
>>>>>1:Operation(1301:JsonObject)
>>>>>>0:Literal(a)
>>>>>>1:Operation(1300:Grouping)
>>>>>>>-1:Literal(33.000000)
>>>>>>0:Literal(b)
>>>>>>1:Operation(1300:Grouping)
>>>>>>>-3:Literal(34.000000)
>>>>0:Literal(z)
>>>>1:Operation(1300:Grouping)
>>>>>1:Operation(1301:JsonObject)
>>>>>>0:Literal(c)
>>>>>>1:Operation(1300:Grouping)
>>>>>>>-1:Literal(35.000000)
>>>>>>0:Literal(d)
>>>>>>1:Operation(1300:Grouping)
>>>>>>>-8:Literal(36.000000)
)Parser");
    }
    SECTION("While statement"){
        is.str("while(x != 'yolo'){ x = 'yolo'; } var z = x;");
        auto tree = parser.parse();

        os << '\n' << tree;
        CHECK(os.str() == R"Parser(
1:Statement(0:TranslationUnit)
>1:Statement(4:While)
>>1:Operation(a01:Inequality)
>>>0:VarUse(name:x)
>>>-1:Literal(yolo)
>>1:Statement(2:Block)
>>>1:Statement(1:Expression)
>>>>1:Operation(300:Assignment)
>>>>>0:VarUse(name:x)
>>>>>-4:Literal(yolo)
>1:Statement(1:Expression)
>>1:VarDecl(name:z)
>>>-4:VarUse(name:x)
)Parser");
    }
    SECTION("DoWhile statement"){
        is.str("do{ x = 'yolo'; } while(x != 'yolo'); var z = x;");
        auto tree = parser.parse();

        os << '\n' << tree;
        CHECK(os.str() == R"Parser(
1:Statement(0:TranslationUnit)
>1:Statement(5:DoWhile)
>>1:Statement(2:Block)
>>>1:Statement(1:Expression)
>>>>1:Operation(300:Assignment)
>>>>>0:VarUse(name:x)
>>>>>-3:Literal(yolo)
>>1:Operation(a01:Inequality)
>>>0:VarUse(name:x)
>>>-2:Literal(yolo)
>1:Statement(1:Expression)
>>1:VarDecl(name:z)
>>>-4:VarUse(name:x)
)Parser");
    }
    SECTION("Function operation"){
        is.str("var g = function(x){ x += 2; return x; }; g(3);");
        auto tree = parser.parse();

        os << '\n' << tree;
        CHECK(os.str() == R"Parser(
1:Statement(0:TranslationUnit)
>1:Statement(1:Expression)
>>1:VarDecl(name:g)
>>>1:Operation(1303:Function)
>>>>0:Literal(undefined)
>>>>0:Literal(x)
>>>>1:Statement(2:Block)
>>>>>1:Statement(1:Expression)
>>>>>>1:Operation(301:AdditionAssignment)
>>>>>>>0:VarUse(name:x)
>>>>>>>-2:Literal(2.000000)
>>>>>1:Statement(7:Return)
>>>>>>-5:VarUse(name:x)
>1:Statement(1:Expression)
>>1:Operation(1100:Call)
>>>0:VarUse(name:g)
>>>-4:Literal(3.000000)
)Parser");
    }
    SECTION("Function statement"){
        is.str("function g(x){ x += 2; return x; } g(3);");
        auto tree = parser.parse();

        os << '\n' << tree;
        CHECK(os.str() == R"Parser(
1:Statement(0:TranslationUnit)
>1:Statement(8:Function)
>>1:Operation(1303:Function)
>>>0:Literal(g)
>>>0:Literal(x)
>>>1:Statement(2:Block)
>>>>1:Statement(1:Expression)
>>>>>1:Operation(301:AdditionAssignment)
>>>>>>0:VarUse(name:x)
>>>>>>-2:Literal(2.000000)
>>>>1:Statement(7:Return)
>>>>>-4:VarUse(name:x)
>1:Statement(1:Expression)
>>1:Operation(1100:Call)
>>>0:VarUse(name:g)
>>>-4:Literal(3.000000)
)Parser");
    }
}

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
3.000000
)Interpreter");
    }
    SECTION("Variable declaration"){
        is.str("var x = 3; var y; var z = x;");
        auto tree = parser.parse();
        interpreter.feed(tree);

        os << '\n' << interpreter.execute() << '\n';
        CHECK(os.str() == R"Interpreter(
3.000000
)Interpreter");
    }
    SECTION("JsonObject simple"){
        is.str("var x = {}; var y = {'abc':'nooo', 34:42, x};");
        auto tree = parser.parse();
        interpreter.feed(tree);

        os << '\n' << interpreter.execute() << '\n';
        CHECK(os.str() == R"Interpreter(
{"x":{},"abc":"nooo","34.000000":42.000000}
)Interpreter");
    }
}
