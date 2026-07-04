#include "doctest/doctest.h"
#include "../../DataToTDFormat/TDFormatConverterSystem.h"
#include "DataTypesAndTheirWidgets/DataTypesAndTheirWidgets.h"

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
	SV_WARN("Hello im here");

	CHECK(checkEquals(LimitedInt(10, 0, 100), { 10,0,0,0 }));

	CHECK(checkEquals(	LimitedDoubleVec
						{ 
							LimitedDouble(10, 0, 100), 
							LimitedDouble(20, 0, 100), 
							LimitedDouble(30, 0, 100)
						}, 
						SUP_Vec4{ 10,20,30,0 }) );

	CHECK(checkEquals(	EnumVec
						{ 
							Enum({{10, "a"}, {20, "b"}}, 1),
							Enum({{5,  "a"}, {6,  "b"}}, 0)
						}, 
						SUP_Vec4{ 20,5,0,0 }) );
}