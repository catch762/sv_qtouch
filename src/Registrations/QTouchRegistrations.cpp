#include "QTouchRegistrations.h"
#include "../DataToTDFormat/TDFormatConverterSystem.h"
#include "DataTypesAndTheirWidgets/DataTypesAndTheirWidgets.h"

//this better be included once, in the cpp file where we do registrations (here)
#include "TDFormatConverters.h"

void QTouchAppTypesMetadata::registerEverything()
{
	registerTDFormatConverters();

    everythingRegisteredFlagRef() = true;
}

bool QTouchAppTypesMetadata::everythingWasRegistered()
{
    return everythingRegisteredFlagRef();
}

bool& QTouchAppTypesMetadata::everythingRegisteredFlagRef()
{
    static bool flag = false;
    return flag;
}

void QTouchAppTypesMetadata::registerTDFormatConverters()
{
	TDFormatConverterSystem::registerConverterForType<LimitedInt>();
	TDFormatConverterSystem::registerConverterForType<LimitedIntVec>();
	TDFormatConverterSystem::registerConverterForType<LimitedDouble>();
	TDFormatConverterSystem::registerConverterForType<LimitedDoubleVec>();
	TDFormatConverterSystem::registerConverterForType<Enum>();
	TDFormatConverterSystem::registerConverterForType<EnumVec>();
}
