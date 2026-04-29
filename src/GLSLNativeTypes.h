#pragma once
#include "sv_qtcommon.h"

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