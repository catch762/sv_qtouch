#pragma once

class QTouchAppTypesMetadata
{
public:
    static void registerEverything();

    static bool everythingWasRegistered();

private:
    static bool& everythingRegisteredFlagRef();
private:
    static void registerTDFormatConverters();
};