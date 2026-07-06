#pragma once
#include "sv_qtcommon.h"
#include "SUP_ArglistParser.h"

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

//SUP-conforming GLSL code may have comments containg some named constants,
//that later on you may refer to in uiMacroString. All these variables are
//saved to this VariablesDict:
struct SUP_VariablesDict
{
    const SUP_Expr* getVariableExpr(const QString& name) const
    {
        return getValue(dict, name);
    }

    //returns nullopt for success
    StringErrOpt saveVariableExpr(const QString& name, SUP_Expr expr)
    {
        if (getValue(dict, name))
        {
            return std::format("saveVariableExpr error: key [{}] already exists in VariablesDict!", name);
        }

        if (auto notFoundNames = replaceAllSymbolTokensWithDictEntries(expr))
        {
            return std::format("saveVariableExpr error: cant save new dict var [{}] because it used following dict vars that were not found: {}",
                                name, *notFoundNames);
        }

        dict[name] = std::move(expr);
        return {};
    }

    std::string toString(bool verbose = true) const
    {
        std::string res = "SUP_VariablesDict{\n";
        for (auto &item : dict)
        {
            std::string itemText = verbose ? item.second.toString() : (item.second.isLeaf() ? "leaf" : "arr");
            res += std::format("  '{}': {}\n", item.first, itemText);
        }
        res += "}";
        return res;
    };

    //return nullopt if success, or set of not found var names if faliure
    StringSetOpt replaceAllSymbolTokensWithDictEntries(SUP_Expr& expr) const
    {
        std::set<std::string> notFoundNames;

        expr.visit([&](SUP_Expr& node)
        {
            if(auto* token = node.getLeafValue())
            {
                if (token->isSymbol())
                {
                    const QString variableName = QString::fromStdString(token->getSymbolData());
                    if (const SUP_Expr* exprFromDict = getVariableExpr(variableName))
                    {
                        //So, this token contains data:
                        //  - variable name
                        //  - flag that indicates we must take negative of that variable value.
                        if (token->isNegative())
                        {
                            //we can only apply it to numbers.
                            
                        }

                        const bool mustNegateValue = token->isNegative();

                        //Ok, we copied it here. 'token' is invalid ptr now btw.
                        node = *exprFromDict;

                        if (mustNegateValue)
                        {
                            if (node.isComposite() || !node.getLeafValue()->isNumber())
                            {
                                SV_MSGBOX_ERROR(std::format("there is minus sign before dict var [{}], which is unexpectedly non-number expression, its [{}]\n\n"
                                                            "Cant negate it, and this negation will be ignored",
                                                            variableName, node));
                            }
                            else
                            {
                                node.getLeafValue()->negateNumber();
                            }
                        }
                    }
                    else
                    {
                        notFoundNames.emplace(variableName.toStdString());
                    }
                }
            }

            return true;
        });

        if (notFoundNames.empty()) return {}; //success
        else return notFoundNames;
        //else return std::format("SUP_VariablesDict couldnt replace all symbols with dict entries, following entries were not found: {}", notFoundNames);
    }

    std::map<QString, SUP_Expr> dict;
};
SV_DECL_STD_FORMATTER(SUP_VariablesDict, obj.toString());

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

    SUP_VariablesDict varDict;

    std::string toString() const;
    const SUP_StructDefinition* getStruct(const QString& name) const; //returns nullptr if not found
};
SV_DECL_OPT(SUP_Data);
SV_DECL_STD_FORMATTER(SUP_Data, obj.toString());