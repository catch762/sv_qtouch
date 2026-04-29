#include "GLSLNativeTypes.h"
#include "DataTypesAndTheirWidgets/DataTypesAndTheirWidgets.h"

QtTypeIndexOpt GLSLNativeTypes::getTypeIndex(const QString &glslTypeName)
{
    return getValueOpt(instance().glslTypeNameToCppType, glslTypeName);
}

GLSLNativeTypes& GLSLNativeTypes::instance()
{
    static GLSLNativeTypes inst;
    return inst;
}

GLSLNativeTypes::GLSLNativeTypes()
{
    addMapping<LimitedDouble>   ("float");
    addMapping<LimitedDoubleVec>("vec4");
}
