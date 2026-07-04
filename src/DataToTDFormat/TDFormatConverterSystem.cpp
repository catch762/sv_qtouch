#include "TDFormatConverterSystem.h"
#include "TDFormatConverters.h"

TDFormatConverterSystem::TDFormatConverterSystem()
{
	registerConverterForType<LimitedInt>();
	registerConverterForType<LimitedIntVec>();
	registerConverterForType<LimitedDouble>();
	registerConverterForType<LimitedDoubleVec>();
	registerConverterForType<Enum>();
	registerConverterForType<EnumVec>();
}

TDFormatConverterSystem& TDFormatConverterSystem::instance()
{
	static TDFormatConverterSystem system;
	return system;
}
