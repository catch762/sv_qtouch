#include "../SUP_NativeGLSLTypeConverter.h"
#include "doctest/doctest.h"


std::string contextInfo(const QString& typeName, const QStringOpt& uiMacroString = {})
{
    return uiMacroString ?  std::format("Converting from type name [{}] and uiMacroString[{}]", typeName, uiMacroString.value()):
                            std::format("Converting from type name [{}]", typeName);
}

TEST_CASE("Converting types from just name, no uimacrostring")
{
    const auto& conv = SUP_NativeGLSLTypeConverter::instance();

    SUBCASE("Converting float")
    {
        auto result = conv.convert("float", {});
        REQUIRE(result);
        REQUIRE(result->variantHoldsType<LimitedDouble>());
    }
    SUBCASE("Converting int")
    {
        auto result = conv.convert("int", {});
        REQUIRE(result);
        REQUIRE(result->variantHoldsType<LimitedInt>());
    }

    SUBCASE("Converting vec2")
    {
        auto result = conv.convert("vec2", {});
        REQUIRE(result);
        REQUIRE(result->variantHoldsType<LimitedDoubleVec>());
        CHECK(result->value.value<LimitedDoubleVec>().size() == 2);
    }
    SUBCASE("Converting ivec2")
    {
        auto result = conv.convert("ivec2", {});
        REQUIRE(result);
        REQUIRE(result->variantHoldsType<LimitedIntVec>());
        CHECK(result->value.value<LimitedIntVec>().size() == 2);
    }
    SUBCASE("Converting vec3")
    {
        auto result = conv.convert("vec3", {});
        REQUIRE(result);
        REQUIRE(result->variantHoldsType<LimitedDoubleVec>());
        CHECK(result->value.value<LimitedDoubleVec>().size() == 3);
    }
    SUBCASE("Converting ivec3")
    {
        auto result = conv.convert("ivec3", {});
        REQUIRE(result);
        REQUIRE(result->variantHoldsType<LimitedIntVec>());
        CHECK(result->value.value<LimitedIntVec>().size() == 3);
    }
    SUBCASE("Converting vec4")
    {
        auto result = conv.convert("vec4", {});
        REQUIRE(result);
        REQUIRE(result->variantHoldsType<LimitedDoubleVec>());
        CHECK(result->value.value<LimitedDoubleVec>().size() == 4);
    }
    SUBCASE("Converting ivec4")
    {
        auto result = conv.convert("ivec4", {});
        REQUIRE(result);
        REQUIRE(result->variantHoldsType<LimitedIntVec>());
        CHECK(result->value.value<LimitedIntVec>().size() == 4);
    }
}

TEST_CASE("Converting scalar types from name AND uimacrostring")
{
    const auto& conv = SUP_NativeGLSLTypeConverter::instance();

    SUBCASE("Converting float with uimacrostring")
    {
        auto result = conv.convert("float", "[10, 30.5, 20.75]");
        REQUIRE(result);
        REQUIRE(result->variantHoldsType<LimitedDouble>());
        auto actualValue = result->value.value<LimitedDouble>();
        auto expectValue = LimitedDouble(20.75, 10.0, 30.5);

        INFO(std::format("expected {} != actual {}", expectValue, actualValue));
        CHECK(expectValue == actualValue);
    }
    SUBCASE("Converting int with uimacrostring")
    {
        auto result = conv.convert("int", "[-100, 100, 0]");
        REQUIRE(result);
        REQUIRE(result->variantHoldsType<LimitedInt>());
        auto actualValue = result->value.value<LimitedInt>();
        auto expectValue = LimitedInt(0, -100, 100);

        INFO(std::format("expected {} != actual {}", expectValue, actualValue));
        CHECK(expectValue == actualValue);
    }
}

TEST_CASE("Converting vector types from name AND uimacrostring")
{
    const auto& conv = SUP_NativeGLSLTypeConverter::instance();

    SUBCASE("Converting vec2 with uimacrostring")
    {
        auto result = conv.convert("vec2", "[[-500, -600, -550], [0,2,1]]");
        REQUIRE(result);
        REQUIRE(result->variantHoldsType<LimitedDoubleVec>());
        auto actualValue = result->value.value<LimitedDoubleVec>();
        auto expectValue = LimitedDoubleVec{
            LimitedDouble(-550, -500, -600),
            LimitedDouble(1, 0, 2)
        };

        INFO(std::format("expected {} != actual {}", expectValue, actualValue));
        CHECK(expectValue == actualValue);
    }

    SUBCASE("Converting ivec3 with uimacrostring")
    {
        auto result = conv.convert("ivec3", "[[-500, -600, -550], [0,2,1], [7,9,8]]");
        REQUIRE(result);
        REQUIRE(result->variantHoldsType<LimitedIntVec>());
        auto actualValue = result->value.value<LimitedIntVec>();
        auto expectValue = LimitedIntVec{
            LimitedInt(-550, -500, -600),
            LimitedInt(1, 0, 2),
            LimitedInt(8, 7, 9)
        };

        INFO(std::format("expected {} != actual {}", expectValue, actualValue));
        CHECK(expectValue == actualValue);
    }

    SUBCASE("Converting vec4 with uimacrostring, which has 3 arrays")
    {
        auto result = conv.convert("vec4", "[[-500, -600, -550], [0,2,1], [7,9,8]]");
        REQUIRE(result);
        REQUIRE(result->variantHoldsType<LimitedDoubleVec>());
        auto actualValue = result->value.value<LimitedDoubleVec>();
        auto expectValue = LimitedDoubleVec{
            LimitedDouble(-550, -500, -600),
            LimitedDouble(1, 0, 2),
            LimitedDouble(8, 7, 9),
            LimitedDouble(8, 7, 9)
        };

        INFO(std::format("expected {} != actual {}", expectValue, actualValue));
        CHECK(expectValue == actualValue);
    }
}

TEST_CASE("Converting to enums")
{
    const auto& conv = SUP_NativeGLSLTypeConverter::instance();

    SUBCASE("Converting int to enum with rad")
    {
        auto result = conv.convert("int", "rad = [10, 'hi', 'kek', 100, >, 'uhh', 'ehh']");
        REQUIRE(result);
        REQUIRE(result->variantHoldsType<Enum>());
        auto actualValue = result->value.value<Enum>();
        auto expectValue = Enum{
            {
                {10, "hi"},
                {11, "kek"},
                {100, "uhh"},
                {101, "ehh"}
            },
            2
        };

        INFO(std::format("expected {} != actual {}", expectValue, actualValue));
        CHECK(expectValue == actualValue);
    }

    SUBCASE("Converting vec4 to enum with rad")
    {
        auto result = conv.convert("ivec4", "rad = [['yo'], [10, 'hi', 'kek', 100, >, 'uhh', 'ehh']]");
        REQUIRE(result);
        REQUIRE(result->variantHoldsType<EnumVec>());
        auto actualValue = result->value.value<EnumVec>();

        auto last3items =   Enum
                            {
                                {
                                    {10, "hi"},
                                    {11, "kek"},
                                    {100, "uhh"},
                                    {101, "ehh"}
                                },
                                2
                            };

        auto expectValue = EnumVec{
            Enum
            {
                {
                    {0, "yo"}
                },
                0
            },
            last3items,
            last3items,
            last3items
        };

        INFO(std::format("expected {} != actual {}", expectValue, actualValue));
        CHECK(expectValue == actualValue);
    }
}