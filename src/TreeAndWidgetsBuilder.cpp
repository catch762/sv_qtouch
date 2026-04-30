#include "TreeAndWidgetsBuilder.h"
#include "GLSLNativeTypes.h"
#include "SerializationLogic/SerializationSystem.h"
#include "WidgetLogic/WidgetMakerSystem.h"

NodeAndWidgetPairOpt TreeAndWidgetsBuilder::buildTreeAndWidgetsForVariable(const SUP_Data &data, const SUP_Variable &var)
{
    if (SUP_VariableConversion::instance().hasConverterForType(var.type))
    {
        auto res = SUP_VariableConversion::instance().convert(var.type, var.uiMacroArg);

        if (!res)
        {
            //SV_ERROR("During building widgets, SUP_VariableConversion returned error. Aborting.");
            return {};
        }

        auto node = DataNode::makeLeaf(var.name, res->value);
        auto widget = WidgetMakerSystem::instance().createAndRegisterWidgetForNode(node, res->jsonForWidget);
        if (!qVariantHasWidget(widget))
        {
            //SV_ERROR("During building widgets, failed to make widget for node. Aborting.");
            return {};
        }
        else getWidgetFromQVariant(widget)->show();

        return NodeAndWidgetPair{node, widget};
    }
    else
    {
        SV_WARN("no conv for " + var.type.toStdString());
        return {};
    }
}
