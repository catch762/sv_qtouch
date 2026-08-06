#include "TreeAndWidgetsBuilder.h"
#include "SUP_Data/SUP_NativeGLSLTypeConverter.h"
#include "SerializationLogic/SerializationSystem.h"
#include "WidgetLogic/WidgetMakerSystem.h"
#include "WidgetLogic/NodeWidget.h"
#include "WidgetLogic/WidgetsForNodeManager.h"
#include "QTouchDefs.h"

TreeAndTopLevelWidgetsOpt TreeAndWidgetsBuilder::buildTreeAndWidgets(const SUP_Data &data)
{
    NodeAndWidgetPairList topLevelItems;

    int entryIdx = 0;
    for (auto &varlistEntry : data.varListEntries)
    {
        auto nodeAndWidget = buildTreeAndWidgetsForVariable(data, varlistEntry);
        if (!nodeAndWidget || !nodeAndWidget->isValid())
        {
            SV_ERROR(std::format("Failed building valid NodeAndWidgetPairList for entry [{}], {}", entryIdx, varlistEntry));
            return {};
        }

        topLevelItems.push_back(*nodeAndWidget);

        entryIdx++;
    }

    if (topLevelItems.empty())
    {
        SV_WARN("TreeAndWidgetsBuilder finished operation with 0 items created");
        return {};
    }

    DataNodeShared root = DataNode::makeComposite();
    NodeWidgetVec topLevelWidgets;

    for(auto &nodeAndWidget : topLevelItems)
    {
        root->addChild(nodeAndWidget.node);
        topLevelWidgets.push_back(nodeAndWidget.widget);
    }

    return TreeAndTopLevelWidgets{std::move(root), std::move(topLevelWidgets)};
}

NodeAndWidgetPairOpt TreeAndWidgetsBuilder::buildTreeAndWidgetsForVariable(const SUP_Data &data, const SUP_Variable &var)
{
    if (SUP_NativeGLSLTypeConverter::instance().hasConverterForType(var.type)) //THEN ITS A NATIVE TYPE SO WE JUST MAKE IT
    {
        auto res = SUP_NativeGLSLTypeConverter::instance().convert(var.type, var.arglistFromUiMacro);
        if (!res)
        {
            SV_ERROR(std::format("buildTreeAndWidgetsForVariable: failed to convert seemingly native {} to C++ value", var));
            return {};
        }

        if (!addTabInformationIfNeeded(res->jsonForWidget, var.arglistFromUiMacro))
        {
            return {};
        }

        auto node = DataNode::makeLeaf(var.name, res->value);
        SV_ASSERT(node);

        auto widget = WidgetMakerSystem::instance().createAndRegisterWidgetForNode(node, res->jsonForWidget);
        if (!widget)
        {
            SV_ERROR(std::format("buildTreeAndWidgetsForVariable: failed to make widget for {} representing seemingly native {}", node, var));
            return {};
        }
        //else widget->show(); //todo check if this is needed -- HEY I THINK ITS HARMFUL

        return NodeAndWidgetPair{node, widget};
    }
    else //THEN ITS A STRUCT, SO WE BUILD IT RECOURSIVELY
    {
        auto node = DataNode::makeComposite(var.name);

        auto structDefinition = data.getStruct(var.type);
        if (!structDefinition)
        {
            SV_ERROR(std::format("buildTreeAndWidgetsForVariable: failed, no struct definition exists for {}", var));
            return {};
        }

        NodeWidgetVec memberWidgets;

        for (const auto& member : structDefinition->members)
        {
            auto memberNodeAndWidget = buildTreeAndWidgetsForVariable(data, member);
            if (!memberNodeAndWidget || !memberNodeAndWidget->isValid())
            {
                SV_ERROR(std::format("buildTreeAndWidgetsForVariable: failed making valid member {} of struct {}", member, *structDefinition));
                return {};
            }

            node->addChild(memberNodeAndWidget->node);
            memberWidgets.push_back(memberNodeAndWidget->widget);
        }

        QJsonObjectWithWidgetOptionsOpt widgetOptionsOpt = {};
        if (!addTabInformationIfNeeded(widgetOptionsOpt, var.arglistFromUiMacro))
        {
            return {};
        }

        auto *finalWrapperWidget = NodeWidget::makeNodeWidgetForCompositeNode(memberWidgets, node, var.name, widgetOptionsOpt);
        WidgetsForNodeManager::registerWidgetForNode(node, finalWrapperWidget);


        return NodeAndWidgetPair{node, finalWrapperWidget};
    }
}

bool TreeAndWidgetsBuilder::addTabInformationIfNeeded(QJsonObjectWithWidgetOptionsOpt& outWidgetOptionsOpt, const SUP_ArglistOpt& arglistFromMacroString)
{
    if (arglistFromMacroString)
    {
        if (const SUP_Expr* tabArg = arglistFromMacroString->getArgByName("tab"))
        {
            if (!tabArg->isLeaf() || !tabArg->getLeafValue()->isNumberInt())
            {
                SV_ERROR(std::format("addTabInformationIfNeeded failed: tabArg isnt NumberInt leaf: {}", *tabArg));
                return false;
            }

            TabIndex tabIndex = tabArg->getLeafValue()->getNumberIntData();

            //we dont need to save default val
            if (tabIndex == 1)
            {
                return true;
            }

            if (!outWidgetOptionsOpt)
            {
                outWidgetOptionsOpt = QJsonObjectWithWidgetOptions();
            }

            (*outWidgetOptionsOpt)[NodeWidget::tabIndexKey] = tabIndex;
        }
    }

    return true;
}