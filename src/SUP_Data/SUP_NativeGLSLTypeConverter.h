#pragma once
#include "sv_qtcommon.h"

#include "DataTypesAndTheirWidgets/DataTypesAndTheirWidgets.h"
#include "SUP_ArglistParser.h"

//******************************************************************************************************
//
// So, SUP-conforming GLSL code has lines like these:
//      VAR(vec4, hello) ui("[10, 30, 20]")
//
// These are first parsed into simple struct SUP_Variable{type, name, uiMacroString}
//
// What this means is:
//      We have variable of GLSL type 'vec4' and name 'hello' and there is arbitrary text data
//      called uiMacroString, which in this case means "when widget is created, default value
//      must be 20, and min is 10 and max is 30". The uiMacroString may potentially contain
//      information about both value and widget representing the value.
//
// So, what this class does is it takes pairs of {glslTypeName, uiMacroString} of each such variable,
// and it produces Output for them, which is:
//      - C++ type variable that represents this GLSL type, with appropriate initial value. Its wrapped in std::any.
//      - Optional JSON with additional widget options, because uiMacroString may contain them.
//        You supply this JSON when you create the widget.
//
// The reason i chose to output this data specifically, is because WidgetMakerSystem needs this data
// to create the widget (std::any with any supported type, and JSON options for the widget)
//
// [!] This class only works with NATIVE scalar GLSL types, like float or ivec4.
// It does not handle structs.
// It also doesnt handle bool, because who even needs this type, use int.
//
//******************************************************************************************************
class SUP_NativeGLSLTypeConverter
{
public:
    struct Output
    {
        std::any value;
        QJsonObjectWithWidgetOptionsOpt jsonForWidget;

        template<typename T>
        bool variantHoldsType()
        {
            return anyHoldsType<T>(value);
        }
    };
    SV_DECL_OPT(Output);

    using Converter = std::function<OutputOpt(const QStringOpt& uiMacroString, const SUP_VariablesDict* dict)>;

    OutputOpt convert(const QString& glslTypeName, const QStringOpt& uiMacroString, const SUP_VariablesDict* dict = nullptr) const
    {
        if (auto* converter = getValue(converters, glslTypeName))
        {
            return (*converter)(uiMacroString, dict);
        }
        else
        {
            SV_ERROR(std::format("SUP_NativeGLSLTypeConverter: did not find converter for glslTypeName={}", glslTypeName));
            return {};
        }
    }

    void registerConverter(const QString& glslTypeName, Converter converter)
    {
        if (converters.contains(glslTypeName))
        {
            SV_WARN(std::format("SUP_NativeGLSLTypeConverter: overwriting Converter for glslTypeName={}", glslTypeName));
        }

        converters[glslTypeName] = converter;
    }

    static SUP_NativeGLSLTypeConverter& instance()
    {
        static SUP_NativeGLSLTypeConverter res;
        return res;
    }

    bool hasConverterForType(const QString& glslTypeName)
    {
        return converters.contains(glslTypeName);
    }

private:
    SUP_NativeGLSLTypeConverter()
    {
        registerConverter("float",  convertIntsAndFloats<double, 1>);
        registerConverter("vec2",   convertIntsAndFloats<double, 2>);
        registerConverter("vec3",   convertIntsAndFloats<double, 3>);
        registerConverter("vec4",   convertIntsAndFloats<double, 4>);

        registerConverter("int",    convertIntsAndFloats<int, 1>);
        registerConverter("ivec2",  convertIntsAndFloats<int, 2>);
        registerConverter("ivec3",  convertIntsAndFloats<int, 3>);
        registerConverter("ivec4",  convertIntsAndFloats<int, 4>);
    }

    DISABLE_COPY_AND_ASSIGNMENT(SUP_NativeGLSLTypeConverter);

    //arrayExpr must hold exactly 3 chidren containing NumberInt or NumberDouble 
    //tokens (type must match UnderlyingType parameter)
    template<StrictlyIntOrDouble UnderlyingType>
    static std::optional<std::array<UnderlyingType, 3>> getThreeNumbers(const SUP_Expr& arrayExpr)
    {
        if (!arrayExpr.isComposite() || arrayExpr.getChildren()->size() != 3)
        {
            return {};
        }

        const auto& arrItems = *arrayExpr.getChildren();

        std::array<UnderlyingType, 3> result;
        if constexpr (std::is_same_v<UnderlyingType, int>)
        {
            for (int i = 0; i < 3; ++i)
            {
                const auto arrItem = arrItems[i];
                if (!arrItem.isLeaf()) return {};
                if(!arrItem.getLeafValue()->isNumberInt()) return {};

                result[i] = arrItem.getLeafValue()->getNumberIntData();
            }
            return result;
        }
        else //double
        {
            for (int i = 0; i < 3; ++i)
            {
                const auto arrItem = arrItems[i];
                if (!arrItem.isLeaf()) return {};
                if(!arrItem.getLeafValue()->isNumber()) return {}; //we accept both NumberInt and NumberDouble tokens, just cast it to double

                result[i] = arrItem.getLeafValue()->getNumberDataAsDouble();
            }
            return result;
        }
    }

    //arrayData is expected to be:
    //    - [array of whatever which isnt array]
    //          -> we just return 'arrayData'
    //    - [array of up to 4 arrays of [whatever]]
    //          -> we return inner array using preferredArrayIndex, or the last available array
    //
    // Return value is SUP_Expr which likely is flat array with three numbers 
    // (BUT ITS NOT CHECKED), or nullptr if something's clearly wrong.
    static const SUP_Expr* getWhatLooksLikeBestArrayForIndex(const SUP_Expr& arrayData, int preferredArrayIndex)
    {
        SV_ASSERT(preferredArrayIndex >= 0 && preferredArrayIndex <= 3);

        if(!arrayData.isComposite()) return nullptr;

        const auto& children = arrayData.getChildren();
        if (children->empty()) return nullptr;

        const bool looksLikeArrayOfArrays = children->front().isComposite();

        if (looksLikeArrayOfArrays)
        {
            const int actualArraySelected = std::min(preferredArrayIndex, int(children->size()-1));
            return &( (*children)[actualArraySelected] );
        }
        else
        {
            return &arrayData;
        }
    }


    //handles: float vec2 vec3 vec4 int ivec2 ivec3 ivec4
    //uiMacroString is either:
    //  json array of 3 doubles (single int/float)
    //  json array of N json arrays of 3 doubles (vecN, ivecN)
    //      (If N is less than componentCount, its ok - we will use last available array in that case)

    //handles: float vec2 vec3 vec4 int ivec2 ivec3 ivec4
    //uiMacroString is either:
    //  json array of 3 doubles (single int/float)
    //  json array of N json arrays of 3 doubles (vecN, ivecN)
    //      (If N is less than componentCount, its ok - we will use last available array in that case)
    template<StrictlyIntOrDouble UnderlyingType, int componentCount>
    static OutputOpt convertIntsAndFloats(const QStringOpt& uiMacroString, const SUP_VariablesDict* dict)
    {
        if (uiMacroString)
        {
            auto parsedDataOrErr = dict ? SUP_ArglistParser().parseToArglistAndReplaceSymbolTokensWithDictEntries(*uiMacroString, *dict) :
                                          SUP_ArglistParser().parseToArglistWithoutSymbolSubstitutions(*uiMacroString);

            if (auto err = getError(parsedDataOrErr))
            {
                SV_ERROR(std::format("convertIntsAndFloats failed: couldnt parse uiMacroString: {}", *err));
                return {};
            }
            const auto& parsedData = std::get<0>(parsedDataOrErr);

            //note that we get args differently: getArg, getArgByName

            if constexpr(std::is_same_v<UnderlyingType, int>)
            {
                if (const SUP_Expr* radArg = parsedData.getArgByName("rad")) //int, but its for radio button, not slider
                {
                    return convertToEnum<componentCount>(*radArg);
                }
            }

            if (const SUP_Expr* limsArg = parsedData.getArg(0, "lims"))
            {
                return  convertToLimited<UnderlyingType, componentCount>(*limsArg);
            }

            SV_ERROR(std::format("convertIntsAndFloats fails to deal with data: {}", parsedData));
            return {};
        }
        else return convertToLimited<UnderlyingType, componentCount>({});
    }

    template<StrictlyIntOrDouble UnderlyingType, int componentCount>
    static OutputOpt convertToLimited(const SUP_ExprOpt& settings)
    {
        SV_ASSERT(componentCount >= 1 && componentCount <= 4);

        using LimitedT = LimitedValue<UnderlyingType>;

        if (componentCount == 1)
        {
            LimitedT limitedValue;

            if (!settings)
            {
                //return whatever was default-constructed.
                return Output{ std::any(limitedValue) };
            }

            auto threeNumbers = getThreeNumbers<UnderlyingType>(*settings);
            if (!threeNumbers)
            {
                SV_ERROR(std::format("Could not parse three numbers from {}", *settings));
                return {};
            }

            limitedValue = LimitedT{threeNumbers->at(2),
                                    threeNumbers->at(0),
                                    threeNumbers->at(1)};
            return Output{ std::any(limitedValue) };
        }
        else
        {
            using LimitedTVec = std::vector<LimitedT>;
            LimitedTVec limitedVec(componentCount);

            if (!settings)
            {
                //return whatever was default-constructed.
                return Output{ std::any(limitedVec) };
            }

            for (int i = 0; i < componentCount; ++i)
            {
                const auto *arrayForThisComponent = getWhatLooksLikeBestArrayForIndex(*settings, i);
                if (!arrayForThisComponent)
                {
                    SV_ERROR(std::format("Error parsing SUP_Expr with data for LimitedValue: cant even find array in {}", settings.value()));
                    return {};
                }

                auto threeNumbers = getThreeNumbers<UnderlyingType>(*arrayForThisComponent);
                if (!threeNumbers)
                {
                    SV_ERROR(std::format("Error parsing SUP_Expr with data for LimitedValue: cant parse array to numbers {}", *arrayForThisComponent));
                    return {};
                }

                limitedVec[i]   = LimitedT{ threeNumbers->at(2),
                                            threeNumbers->at(0),
                                            threeNumbers->at(1) };
            }

            return Output{ std::any(limitedVec) };
        }
    }

    
    // Array data is array of pairs of OPTIONAL_int(enum value), REQUIRED_string(enum name), e.g. :
    //  { 10, "name for 10", 20, "name for 20", "you didnt specify int value so it will be name for 21",
    //    "this will be for 22", 16, "this will be name for 16" }
    //
    // If you dont put enumValue's at all, like {"a", "b", "c"} - it will use values of 0, 1, 2 by default
    static EnumOpt makeEnum(const SUP_Expr& arrayData)
    {
        if (!arrayData.isComposite())
        {
            SV_ERROR(std::format("Cant make enum ot of leaf {}", arrayData));
            return {};
        };

        const auto& children = arrayData.getChildren();
        if (children->empty())
        {
            SV_ERROR("Cant make enum out of empty array");
            return {};
        }

        std::vector<Enum::EnumEntry> entries;

        int currentEnumValue = 0;

        intOpt indexSelectedBySpecChar = {};

        for (int i = 0; i < children->size(); ++i)
        {
            const SUP_Expr& item = children->at(i);
            if (item.isComposite())
            {
                SV_ERROR("Unexpected composite item in enum list");
                return {};
            }

            if (item.getLeafValue()->isNumberInt())
            {
                currentEnumValue = item.getLeafValue()->getNumberIntData();

                if (i == children->size()-1)
                {
                    SV_ERROR("You cant end enum list with a number, there must be string after");
                    return {};
                }
                else
                {
                    const SUP_Expr& peekNextItem = children->at(i+1);
                    if (peekNextItem.isComposite() || peekNextItem.getLeafValue()->isNumberInt())
                    {
                        SV_ERROR(std::format("Unexpected token {} in enum list after int enumValue token", peekNextItem));
                        return {};
                    }
                }
            }
            else if (item.getLeafValue()->isString())
            {
                entries.push_back( {currentEnumValue, QString::fromStdString(item.getLeafValue()->getStringData())} );
                currentEnumValue++;
            }
            else if (item.getLeafValue()->isSpecialCharachter() && item.getLeafValue()->getSpecialCharachterData() == '>')
            {
                if (i == children->size()-1)
                {
                    SV_ERROR("You cant end enum list with a '>' there must be an entry after selector char");
                    return {};
                }
                else if (indexSelectedBySpecChar.has_value())
                {
                    SV_ERROR("Enum list: second '>' token found, there must be only one to select index");
                    return {};
                }
                else indexSelectedBySpecChar = entries.size(); //for next item
            }
            else
            {
                SV_ERROR(std::format("Unexpected token {} in enum list", item));
                return {};
            }
        }

        Enum result = Enum(std::move(entries), indexSelectedBySpecChar.value_or(0));
        if (!result.isValid())
        {
            SV_ERROR(std::format("Well, invalid Enum was formed by this input: {}", arrayData));
            return {};
        }

        return result;
    }

    template<int componentCount>
    static OutputOpt convertToEnum(const SUP_Expr& settings)
    {
        SV_ASSERT(componentCount >= 1 && componentCount <= 4);

        if (componentCount == 1)
        {
            auto theEnum = makeEnum(settings);
            if (!theEnum)
            {
                SV_ERROR("Failed to makeEnum for single Enum");
                return {};
            }

            return Output{ std::any(std::move(*theEnum)) };
        }
        else
        {
            EnumVec vecOfEnums;
            vecOfEnums.reserve(componentCount);

            for (int i = 0; i < componentCount; ++i)
            {
                const auto *arrayForThisComponent = getWhatLooksLikeBestArrayForIndex(settings, i);
                if (!arrayForThisComponent)
                {
                    SV_ERROR(std::format("Error parsing SUP_Expr with data for Enum: cant even find array in {}", settings));
                    return {};
                }

                auto theEnum = makeEnum(*arrayForThisComponent);
                if (!theEnum)
                {
                    SV_ERROR(std::format("Failed to makeEnum for comp {} of {}-EnumVec", i, componentCount));
                    return {};
                }

                vecOfEnums.push_back(std::move(*theEnum));
            }

            return Output{ std::any(std::move(vecOfEnums)) };
        }
    }

private:
    std::map<QString, Converter> converters;

};