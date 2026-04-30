#pragma once
#include "sv_qtcommon.h"

#include "DataTypesAndTheirWidgets/DataTypesAndTheirWidgets.h"
class GLSLNativeTypes
{
public:
    static QtTypeIndexOpt getTypeIndex(const QString& glslTypeName);

private:
    GLSLNativeTypes();
    DISABLE_COPY_AND_ASSIGNMENT(GLSLNativeTypes);

    static GLSLNativeTypes& instance();

    template<typename T>
    void addMapping(const QString& glslTypeName)
    {
        SV_ASSERT(!glslTypeNameToCppType.contains(glslTypeName));
        SV_ASSERT(qtTypeIsRegistered<T>());

        glslTypeNameToCppType[glslTypeName] = qtTypeId<T>();
    }

private:
    std::map<QString, QtTypeIndex> glslTypeNameToCppType;
};


class SUP_VariableConversion
{
public:
    struct Output
    {
        QVariant value;
        QJsonObjectWithWidgetOptionsOpt jsonForWidget;
    };
    SV_DECL_OPT(Output);

    using Converter = std::function<OutputOpt(const QStringOpt& uiMacroString)>;

    OutputOpt convert(const QString& glslTypeName, const QStringOpt& uiMacroString)
    {
        if (auto converter = getValue(converters, glslTypeName))
        {
            return (*converter)(uiMacroString);
        }
        else
        {
            SV_ERROR(std::format("SUP_VariableConversion: did not find converter for glslTypeName={}", glslTypeName));
            return {};
        }
    }

    void registerConverter(const QString& glslTypeName, Converter converter)
    {
        if (converters.contains(glslTypeName))
        {
            SV_WARN(std::format("SUP_VariableConversion: overwriting Converter for glslTypeName={}", glslTypeName));
        }

        converters[glslTypeName] = converter;
    }

private:
    SUP_VariableConversion()
    {
        registerConverter("float", convertToLimited<double, 1>);
    }

    // array of arrays
    // vec3 
    // lims = [0, 1, 0.5]
    // lims = [ [0, 1, 0.5], [5,7,6] ]

    using ThreeDoubles = std::array<double, 3>;
    SV_DECL_OPT(ThreeDoubles);

    static ThreeDoublesOpt getThreeDoubles(const QJsonValue& array)
    {
        auto logErr = [&]()
        {
            SV_ERROR(std::format("Expected array of 3 doubles, got {}", jsonValueToString(array)));
        };

        if (!array.isArray())
        {
            logErr();
            return {};
        }
        auto arr = array.toArray();
        if (arr.size() != 3)
        {
            logErr();
            return {};
        }

        ThreeDoubles res;
        for (int i = 0; i < 3; ++i)
        {
            auto val = arr[i];
            if (!val.isDouble())
            {
                logErr();
                return {};
            }
            else res[i] = val.toDouble();
        }
        return res;
    }

    //handles: float vec2 vec3 vec4 int ivec2 ivec3 ivec4
    //uiMacroString is either:
    //  json array of 3 doubles (single int/float)
    //  json array of N json arrays of 3 doubles (vecN, ivecN)
    //      (If N is less than componentCount, its ok - we will use last available array in that case)
    template<StrictlyIntOrDouble UnderlyingType, int componentCount>
    static OutputOpt convertToLimited(const QStringOpt& uiMacroString)
    {
        SV_ASSERT(componentCount >= 1 && componentCount <= 4);

        using LimitedT = LimitedValue<UnderlyingType>;
        
        auto logMacroStringErr = [&](const std::string &err)
        {
            SV_ERROR(std::format("Failed parsing uiMacroString: {}. uiMacroString = {}", err, uiMacroString));
        };

        auto jsonArray = jsonStringToArray(*uiMacroString);
        if (!jsonArray || jsonArray->empty())
        {
            logMacroStringErr("couldnt get array");
            return {};
        }
        const bool isArrayOfArrays = jsonArray->at(0).isArray();

        if (componentCount == 1)
        {
            LimitedT limitedValue;

            if (!uiMacroString)
            {
                //return whatever was default-constructed.
                return Output{ QVariant::fromValue(limitedValue) };
            }

            auto threeDoubles = getThreeDoubles(*jsonArray);
            if (!threeDoubles)
            {
                logMacroStringErr("couldnt get component array");
                return {};
            }

            limitedValue = LimitedT{threeDoubles->at(0), threeDoubles->at(1), threeDoubles->at(2)};
            return Output{ QVariant::fromValue(limitedValue) };
        }
        else
        {
            using LimitedTVec = std::vector<LimitedT>;
            LimitedTVec limitedVec(componentCount);

            if (!uiMacroString)
            {
                //return whatever was default-constructed.
                return Output{ QVariant::fromValue(limitedVec) };
            }

            for (int i = 0; i < componentCount; ++i)
            {
                auto idx = std::min(i, int(jsonArray->size()-1));
                auto arrayForThisComponent = isArrayOfArrays ?  jsonArray->at( idx ).toArray() :
                                                                *jsonArray;
                auto threeDoubles = getThreeDoubles(arrayForThisComponent);
                if (!threeDoubles)
                {
                    logMacroStringErr("couldnt get component array");
                    return {};
                }

                limitedVec[i] = LimitedT{threeDoubles->at(0), threeDoubles->at(1), threeDoubles->at(2)};
            }

            return Output{ QVariant::fromValue(limitedVec) };
        }
    }

private:
    std::map<QString, Converter> converters;

};