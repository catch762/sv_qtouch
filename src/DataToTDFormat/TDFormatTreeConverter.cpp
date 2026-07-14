#include "TDFormatTreeConverter.h"
#include "TDFormatConverterSystem.h"

StringErrOpt convertTreeToTDFormat(DataNodeShared tree, TreeAsVec4Array& result)
{
    SV_ASSERT(tree);

    result.clear();

    StringErrOpt currentError;

    DataNode::iterateRecoursively(tree, [&](const DataNodeShared& node)
    {
        if (node->isComposite() || currentError.has_value()) return;

        SUP_Vec4Opt convertedVar = TDFormatConverterSystem::convert(*node->tryGetLeafvalue());
        if (!convertedVar)
        {
            currentError = std::format("Failed to convert to tdformat leaf value of {}", node);
            result.clear();
            return;
        }

        result.push_back(*convertedVar);
    });

    return currentError;
}
