#pragma once
#include "sv_qtcommon.h"

//******************************************************************************************************
// These structs represent all the data we obtain from parsing GLSL code.
//
// 'SUP', or Svvvv Uniform Protocol is my small lib for GLSL - a collection of macros, mostly.
// It does not belong in this project, but for a reference, i copied it to /doc/sup.glsl 
// 
// When we parse data from GLSL, we are looking for specific macros defined in SUP.
// Hence i named the structs SUP_Something, rather than GLSL_Something - it reflects the intent better.
//******************************************************************************************************

struct SUP_Variable
{
    QString type;
    QString name;
    QStringOpt uiMacroArg; //not including quotes

    std::string toString() const;
};
SV_DECL_OPT(SUP_Variable);
SV_DECL_STD_FORMATTER(SUP_Variable, obj.toString());

struct SUP_StructDefinition
{
    QString name;
    std::vector<SUP_Variable> members;

    std::string toString() const;
    bool isValid() const;
};
SV_DECL_OPT(SUP_StructDefinition);
SV_DECL_STD_FORMATTER(SUP_StructDefinition, obj.toString());

//********************************************************************
// The final struct, it contains all the data we get from GLSL code.
//********************************************************************
struct SUP_Data
{
    std::map<QString, SUP_StructDefinition>   structDefinitions;

    // Final GLSL project uniform data layout - declaring variables, which are either: 
    //  - of native type such as 'float' or 'vec4'
    //  - a struct from the list of 'structDefinitions'
    std::vector<SUP_Variable>       varListEntries;     

    std::string toString() const;
    const SUP_StructDefinition* getStruct(const QString& name) const; //returns nullptr if not found
};
SV_DECL_OPT(SUP_Data);
SV_DECL_STD_FORMATTER(SUP_Data, obj.toString());