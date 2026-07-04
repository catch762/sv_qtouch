#pragma once
#include "TDFormatConverterInterface.h"
#include "DataTypesAndTheirWidgets/DataTypesAndTheirWidgets.h"

template<StrictlyIntOrDouble UnderlyingT>
class TDFormatConverter< LimitedValue<UnderlyingT> >
{
public:
	using ValT = LimitedValue<UnderlyingT>;
	static SUP_Vec4 convert(const ValT& val)
	{
		SUP_Vec4 vec = {};
		vec[0] = static_cast<float>(val.value());
		return vec;
	}
};

template<StrictlyIntOrDouble UnderlyingT>
class TDFormatConverter< std::vector<LimitedValue<UnderlyingT>> >
{
public:
	using LimValT = LimitedValue<UnderlyingT>;
	using VecT = std::vector<LimValT>;

	static SUP_Vec4 convert(const VecT& val)
	{
		if (val.size() < 1 || val.size() > 4)
		{
			SV_ERROR(std::format("TDFormatConverter for [{}] didnt expect vec size [{}]", typeNameOrMangled<VecT>(), val.size()));
			return {};
		}

		SUP_Vec4 vec = {};

		for (int i = 0; i < val.size(); ++i)
		{
			vec[i] = static_cast<float>(val[i].value());
		}

		return vec;
	}
};

template<>
class TDFormatConverter< Enum >
{
public:
	static SUP_Vec4 convert(const Enum& val)
	{
		intOpt intVal = val.getEnumValue();
		if (!intVal)
		{
			SV_ERROR(std::format("TDFormatConverter cant convert Enum (will use 0), it doesnt have valid value: {}", val));
			return {};
		}

		SUP_Vec4 vec = {};
		vec[0] = static_cast<float>(*intVal);
		return vec;
	}
};

template<>
class TDFormatConverter< EnumVec >
{
public:
	static SUP_Vec4 convert(const EnumVec& val)
	{
		if (val.size() < 1 || val.size() > 4)
		{
			SV_ERROR(std::format("TDFormatConverter for [{}] didnt expect vec size [{}]", typeNameOrMangled<EnumVec>(), val.size()));
			return {};
		}

		SUP_Vec4 vec = {};

		for (int i = 0; i < val.size(); ++i)
		{
			if (auto intVal = val[i].getEnumValue())
			{
				vec[i] = static_cast<float>(*intVal);
			}
			else
			{
				SV_ERROR(std::format("TDFormatConverter cant convert EnumVec's Enum (will use 0) at [{}], it doesnt have valid value: {}", i, val));
			}
		}

		return vec;
	}
};