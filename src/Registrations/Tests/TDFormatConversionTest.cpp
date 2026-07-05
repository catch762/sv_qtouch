#include "doctest/doctest.h"
#include "../../DataToTDFormat/TDFormatConverterSystem.h"
#include "DataTypesAndTheirWidgets/DataTypesAndTheirWidgets.h"
#include "../../DataToTDFormat/TDFormatTreeConverter.h"

bool checkEquals(const auto& input, const SUP_Vec4& expectedOutput)
{
	const SUP_Vec4Opt actualOutput = TDFormatConverterSystem::convert(input);
	if (!actualOutput)
	{
		SV_ERROR("TD Format test checkEquals failed, couldnt even get output");
		return false;
	}

	bool result = arithmeticArraysEquals(expectedOutput, *actualOutput);
	if (!result)
	{
		SV_ERROR(std::format("TD Format test checkEquals failed: expected {} != actual {} for input {}", expectedOutput, *actualOutput, input));
	}

	return result;
}

TEST_CASE("Converting types to vec4 for TD")
{
	CHECK(checkEquals(LimitedInt(10, 0, 100), SUP_Vec4{ 10 }));

	CHECK(checkEquals(	LimitedDoubleVec
						{ 
							LimitedDouble(10, 0, 100), 
							LimitedDouble(20, 0, 100), 
							LimitedDouble(30, 0, 100)
						}, 
						SUP_Vec4{ 10,20,30 }) );

	CHECK(checkEquals(	EnumVec
						{ 
							Enum({{10, "a"}, {20, "b"}}, 1),
							Enum({{5,  "a"}, {6,  "b"}}, 0)
						}, 
						SUP_Vec4{ 20,5 }) );
}

TEST_CASE("Converting entire tree to vec<vec4>")
{
	using namespace datanode_helpers;

	auto tree =	dncomp("root", {
					dncomp("one", {
						dnleaf("a",	LimitedInt{60,50,70}),
						dnleaf("b", LimitedDoubleVec{LimitedDouble{6, 5, 7}, LimitedDouble{-1,0,-1}}),
						dnleaf("c", EnumVec{
										Enum({{10, ""}, {20, ""}, {100, ""}}, 0),
										Enum({{90, ""}, {80, ""}, {700, ""}}, 2),
									})
					}),
					dncomp("two", {
						dnleaf("a", LimitedIntVec{LimitedInt{10, 9, 11}, LimitedInt{0,0,1}}),
						dnleaf("b",	LimitedDouble{6, 5, 7}),
						dnleaf("c",	Enum({{-90, ""}, {-80, ""}, {-700, ""}}, 2))
					})
		});

	TreeAsTDFormatData expectedResult = {
		{60},
		{6, -1},
		{10, 700},
		{10, 0},
		{6},
		{-700}
	};

	TreeAsTDFormatData actualResult;
	auto errOpt = convertTreeToTDFormat(tree, actualResult);
	
	if (errOpt)
	{
		INFO(std::format("convertTreeToTDFormat failed to convert tree with err {}", *errOpt));
		REQUIRE(false);
	}

	if (actualResult != expectedResult)
	{
		FAIL_CHECK(std::format(	"TreeAsTDFormatData test mismatch:\n"
								"EXPECTED: {}\n"
								"ACTUAL:   {}\n", expectedResult, actualResult));

	}
}