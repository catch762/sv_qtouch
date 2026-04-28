#include "SUP_Data.h"

std::string SUP_Variable::toString() const
{
    return std::format("SUP_Var[{}, {}{}]",
        type,
        name,
        uiMacroArg ? std::format(", ui(\"{}\")", uiMacroArg) : std::string("")
    );
}

std::string SUP_StructDefinition::toString() const
{
    auto res = std::format("struct {} {{\n", name);
    for (auto &m : members)
    {
        res += std::format("    {}\n", m);
    }
    res += "}";

    return res;
}

bool SUP_StructDefinition::isValid() const
{
    return !name.isEmpty() && !members.empty();
}

std::string SUP_Data::toString() const
{
    std::string res = std::format(  "SUP_Data BEGIN [\n"
                                    "Struct Definitions ({}):\n", structDefinitions.size());
    for (auto &structDef : structDefinitions)
    {
        res += structDef.second.toString() + "\n";
    }

    res += std::format("--------------\nVarList entries ({}):\n", varListEntries.size());

    for (auto &varlistEntry : varListEntries)
    {
        res += varlistEntry.toString() + "\n";
    }

    res += "] SUP_Data END";
    return res;
}

const SUP_StructDefinition *SUP_Data::getStruct(const QString &name) const
{
    return getValue(structDefinitions, name);
}


