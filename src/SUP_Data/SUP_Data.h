#pragma once
#include "sv_qtcommon.h"

struct SUP_StructMember
{
    QString varType;
    QString varName;
    QStringOpt uiMacroArg; //not including quotes

    std::string toString() const;
};
SV_DECL_OPT(SUP_StructMember);
SV_DECL_STD_FORMATTER(SUP_StructMember, obj.toString());

struct SUP_StructDefinition
{
    QString name;
    std::vector<SUP_StructMember> members;

    std::string toString() const;
    bool isValid() const;
};
SV_DECL_OPT(SUP_StructDefinition);
SV_DECL_STD_FORMATTER(SUP_StructDefinition, obj.toString());

struct SUP_VarListEntry
{
    enum MacroType
    {
        ScalarVariable, //e.g. 'float'
        Struct
    };

    MacroType   macroType;
    QString     varType;
    QString     varName;
    QStringOpt  uiMacroArg;

    std::string toString() const;
};
SV_DECL_OPT(SUP_VarListEntry);
SV_DECL_STD_FORMATTER(SUP_VarListEntry, obj.toString());

struct SUP_Data
{
    std::vector<SUP_StructDefinition>   structDefinitions;
    std::vector<SUP_VarListEntry>       varListEntries;

    std::string toString() const;
};
SV_DECL_OPT(SUP_Data);
SV_DECL_STD_FORMATTER(SUP_Data, obj.toString());