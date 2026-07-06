#include "../SUP_ArglistParser.h"
#include "doctest/doctest.h"

using namespace sup_expr_helpers;
using namespace basictoken_helpers;

SUP_Expr leaf_sym(std::string data)
{
    return leaf(sym(std::move(data)));
}
SUP_Expr leaf_str(std::string data)
{
    return leaf(str(std::move(data)));
}
SUP_Expr leaf_spec(char ch)
{
    return leaf(spec(ch));
}
SUP_Expr leaf_int(int num)
{
    return leaf(mkint(num));
}
SUP_Expr leaf_double(double num)
{
    return leaf(mkdouble(num));
}

//if both hold error, we assume they are equal! even if error is different
bool resultEquals(const SUP_ArglistOrError& first, const SUP_ArglistOrError& second)
{
    if (first.index() != second.index()) return false;
    if (first.index() == 0)
    {
        return std::get<0>(first) == std::get<0>(second);
    }
    else return true;
}

std::string makeContext(const UiMacroString &text, const SUP_ArglistOrError& expected, const SUP_ArglistOrError& actual )
{
    return std::format( "UiMacroString:   {}\n"
                        "Expected result: {}\n"
                        "Actual result:   {}\n", text, variantToString(expected), variantToString(actual));
}



TEST_CASE("Flat list and empty array")
{
    auto text = "a,b,[]";

    SUP_ArglistOrError expectedRes = SUP_Arglist{{
        {"",    leaf_sym("a")},
        {"",    leaf_sym("b")},
        {"",    comp()}
    }};

    auto actualRes = SUP_ArglistParser().parseToArglistWithoutSymbolSubstitutions(text);


    INFO(makeContext(text, expectedRes, actualRes));
    CHECK(resultEquals(expectedRes, actualRes));
}

TEST_CASE("Just an integer")
{
    auto text = "777";

    SUP_ArglistOrError expectedRes = SUP_Arglist{{
        {"",    leaf_int(777)}
    }};

    auto actualRes = SUP_ArglistParser().parseToArglistWithoutSymbolSubstitutions(text);


    INFO(makeContext(text, expectedRes, actualRes));
    CHECK(resultEquals(expectedRes, actualRes));
}

TEST_CASE("Should fail on unexpected comma token")
{
    auto text = "a,b,[,]";

    SUP_ArglistOrError expectedRes = std::string("some error");

    auto actualRes = SUP_ArglistParser().parseToArglistWithoutSymbolSubstitutions(text);


    INFO(makeContext(text, expectedRes, actualRes));
    CHECK(resultEquals(expectedRes, actualRes));
}

TEST_CASE("Long list of everything")
{
    auto text = "a,kek = b, ok = [[x, 'z'], -1.0000]";

    SUP_ArglistOrError expectedRes = SUP_Arglist{{
        {"",    leaf_sym("a")},
        {"kek", leaf_sym("b")},
        {"ok",  comp(
                    comp(
                        leaf_sym("x"),
                        leaf_str("z")
                    ),
                    leaf_double(-1.0)
                )
        }
    }};

    auto actualRes = SUP_ArglistParser().parseToArglistWithoutSymbolSubstitutions(text);


    INFO(makeContext(text, expectedRes, actualRes));
    CHECK(resultEquals(expectedRes, actualRes));
}

TEST_CASE("Should fail because of unexpected token in array")
{
    auto text = "a,kek = b, ok = [[x, 'z'], -1.00 00]";

    SUP_ArglistOrError expectedRes = std::string("some error");

    auto actualRes = SUP_ArglistParser().parseToArglistWithoutSymbolSubstitutions(text);


    INFO(makeContext(text, expectedRes, actualRes));
    CHECK(resultEquals(expectedRes, actualRes));
}

TEST_CASE("Even longer list of everything")
{
    auto text = "arg0 = whatever, lims = [ [0, 1], [2, -3.5, 'hello'] ]";

    SUP_ArglistOrError expectedRes = SUP_Arglist{{
        {"arg0",    leaf_sym("whatever")},
        {"lims",    comp(
                        comp(
                            leaf_int(0),
                            leaf_int(1)
                        ),
                        comp(
                            leaf_int(2),
                            leaf_double(-3.5),
                            leaf_str("hello")
                        )
                    )
        }
    }};

    auto actualRes = SUP_ArglistParser().parseToArglistWithoutSymbolSubstitutions(text);


    INFO(makeContext(text, expectedRes, actualRes));
    CHECK(resultEquals(expectedRes, actualRes));
}