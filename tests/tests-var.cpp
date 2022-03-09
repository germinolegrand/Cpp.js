#include <catch2/catch.hpp>

#include <iostream>

#include "var.h"

TEST_CASE("Var", "[var]"){
    std::ostringstream os;

    var a{{{"d", "e"}}};
    var const& ref = a;
    var copie = a;

    a["b"] = 34.;
    os << '\n' << a << '\n';

    CHECK(os.str() == R"(
{"b":34,"d":"e"}
)");
    CHECK(a["b"] == 34);
    CHECK(ref["b"] == 34);
    CHECK(copie["b"] == a["b"]);

    os.str("");

    var v = 2 < 1;
    os << v;
    CHECK(os.str() == "false");

    var cmp_a = 4.;
    var cmp_b = NAN;
    CHECK((cmp_a < cmp_b) == false);
    var cmp_c = "Not a number";
    CHECK((cmp_a < cmp_c) == false);
}

TEST_CASE("Var with prototype", "[var]"){
    std::ostringstream os;

    var a{{{"foo", "FOO"}}};
    var b{{{"bar", "BAR"}}, a};

    os << b["foo"];
    CHECK(os.str() == "FOO");
}
