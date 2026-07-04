#pragma once
#include "sv_common.h"
#include "TDFormatConverterInterface.h"
#include "DataForTypeMap.h"

class TDFormatConverterSystem
{
public:
	static SUP_Vec4Opt convert(const std::any& any)
	{
		if (const auto* converter = instance().converters.getEntry(typeIndex(any)))
		{
			return (*converter)(any);
		}
		else
		{
			SV_ERROR(std::format("Didnt find TDFormatConverter for {}", any));
			return {};
		}
	}

	using Converter = std::function<SUP_Vec4Opt(const std::any& any)>;

	template<typename T>
	static void registerConverterForType()
	{
		SV_ASSERT(typeIsNamed<T>());

		SV_ASSERT(!instance().converters.entryExistsForEither(typeIndex<T>(), typeName<T>()));

		auto wrappedConverter = [](const std::any& any)->SUP_Vec4Opt
		{
			if (const auto* val = anyGet<T>(any))
			{
				return TDFormatConverter<T>::convert(*val);
			}
			else
			{
				SV_ERROR(std::format("TDFormatConverter expected [{}] in any, found {}", typeName<T>(), any));
				return {};
			}
		};

		instance().converters.addEntryForType<T>(wrappedConverter);
	}

private:
	TDFormatConverterSystem() = default;
	static TDFormatConverterSystem& instance()
	{
		static TDFormatConverterSystem system;
		return system;
	}

	DISABLE_COPY_AND_ASSIGNMENT(TDFormatConverterSystem);

private:
	DataForTypeMap<Converter> converters;
};