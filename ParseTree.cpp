#include "ParseTree.h"

#include <sstream>
#include <catch2/catch.hpp>

TEST_CASE("ParseTree-NoRoot", "[ParseTree]"){
    ParseTree<std::string> tree;
    CHECK(tree.empty() == true);
}

TEST_CASE("ParseTree-Root", "[ParseTree]"){
    ParseTree<std::string> tree("root");
    CHECK(tree.empty() == false);
    CHECK(tree.size() == 1);
    auto root = tree.root();
    CHECK(root.is_root() == true);
    CHECK(root.root() == root);
    CHECK(root.empty() == true);
    CHECK(root.size() == 0);
    CHECK(root.begin() == root.end());
    CHECK(root.deep_weight() == -1);
    CHECK(*root == "root");
    CHECK(root->size() == 4);
    auto const& ctree = tree;
    auto croot = ctree.root();
    CHECK(croot.is_root() == true);
    CHECK(croot.root() == croot);
    CHECK(croot.empty() == true);
    CHECK(croot.size() == 0);
    CHECK(croot.begin() == croot.end());
    CHECK(*croot == "root");
    CHECK(croot->size() == 4);

    CHECK(croot.contains(root) == false);
    CHECK(root.contains(croot) == false);

}

TEST_CASE("ParseTree-BasicTree", "[ParseTree]"){
    ParseTree<std::string> tree("root");
    tree.root().append("child0").append("child1").append("child2");
    std::ostringstream os;
    os << '\n' << tree;
    CHECK(os.str() ==
R"(
1:root
>1:child0
>>1:child1
>>>-4:child2
)");
    os.str("");
    CHECK(*tree.at(0,0,0) == "child2");
    tree.at(0).append("child10");
    os << '\n' << tree;
    CHECK(os.str() ==
R"(
1:root
>1:child0
>>1:child1
>>>-1:child2
>>-3:child10
)");
    os.str("");
}
