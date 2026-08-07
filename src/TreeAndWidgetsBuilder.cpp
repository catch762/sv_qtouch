#include "TreeAndWidgetsBuilder.h"
#include "SUP_Data/SUP_NativeGLSLTypeConverter.h"
#include "SerializationLogic/SerializationSystem.h"
#include "WidgetLogic/WidgetMakerSystem.h"
#include "WidgetLogic/NodeWidget.h"
#include "WidgetLogic/WidgetsForNodeManager.h"
#include "QTouchDefs.h"

DataNodeShared SUP_TreeBuilder::buildTreeAndWidgets(const SUP_Data &data, NodeWidgetVec* outTopLevelWidgets)
{
    NodeAndWidgetPairList topLevelItems;

    int entryIdx = 0;
    for (auto &varlistEntry : data.varListEntries)
    {
        NodeWidgetUnique    widgetUnique;
        NodeWidget* optionalMemberWidget = nullptr;
        auto node = buildTreeForVariable(data, varlistEntry, _placeholder_);
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
        SV_WARN("SUP_TreeBuilder finished operation with 0 items created");
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

DataNodeShared SUP_TreeBuilder::buildTreeForVariable( const SUP_Data&     data,
                                                      const SUP_Variable& var,
                                                      NodeWidgetUnique*   outWidget)
{
    // Note: if im creating widgets, and then encounter error and return --
    // raw Widget* ptrs would remain in memory. So UNTIL i do return successfully,
    // widgets are wrapped in unique ptrs.

    const bool widgetsRequested = static_cast<bool>(outWidget);

    if (SUP_NativeGLSLTypeConverter::instance().hasConverterForType(var.type)) //THEN ITS A NATIVE TYPE SO WE JUST MAKE IT
    {
        auto res = SUP_NativeGLSLTypeConverter::instance().convert(var.type, var.arglistFromUiMacro);
        if (!res)
        {
            SV_ERROR(std::format("buildTreeForVariable: failed to convert seemingly native {} to C++ value", var));
            return {};
        }

        if (!addTabInformationIfNeeded(res->jsonForWidget, var.arglistFromUiMacro))
        {
            return {};
        }

        auto node = DataNode::makeLeaf(var.name, res->value);
        SV_ASSERT(node);


        if (widgetsRequested)
        {
            outWidget->reset( WidgetMakerSystem::instance().createAndRegisterWidgetForNode(node, res->jsonForWidget) );
            if (!outWidget)
            {
                SV_ERROR(std::format("buildTreeForVariable: failed to make widget for {} representing seemingly native {}", node, var));
                return {};
            }
        }

        return node;
    }
    else //THEN ITS A STRUCT, SO WE BUILD IT RECOURSIVELY
    {
        auto node = DataNode::makeComposite(var.name);

        auto structDefinition = data.getStruct(var.type);
        if (!structDefinition)
        {
            SV_ERROR(std::format("buildTreeForVariable: failed, no struct definition exists for {}", var));
            return {};
        }

        QJsonObjectWithWidgetOptionsOpt widgetOptionsOpt = {};
        if (!addTabInformationIfNeeded(widgetOptionsOpt, var.arglistFromUiMacro))
        {
            return {};
        }

        std::vector<NodeWidgetUnique> memberWidgets;

        for (const auto& member : structDefinition->members)
        {
            NodeWidgetUnique    optionalMemberWidgetUnique;
            NodeWidget*         optionalMemberWidget = nullptr;
            auto                memberNode           = buildTreeForVariable(data, member, widgetsRequested ? &optionalMemberWidget : nullptr);
            if (widgetsRequested)
            {
                //as soon as we obtain this widget, we put it to unique ptr, so we dont have to think
                //what happens to it in case of error (unique ptr will clean it up)
                optionalMemberWidgetUnique.reset(optionalMemberWidget);
            }

            const bool  bothObtained    =  memberNode && (widgetsRequested ?  bool(optionalMemberWidgetUnique) : true);
            const bool  noneObtained    = !memberNode && (widgetsRequested ? !bool(optionalMemberWidgetUnique) : true);
            SV_ASSERT(bothObtained || noneObtained);

            if (noneObtained)
            {
                SV_ERROR(std::format("buildTreeForVariable: failed making valid member {} of struct {}", member, *structDefinition));
                return {};
            }

            node->addChild(memberNode);

            if (widgetsRequested)
            {
                memberWidgets.push_back(std::move(optionalMemberWidgetUnique));
            }
        }

        //All good, return both values:
        if (widgetsRequested)
        {
            //this cant fail
            auto* finalWrapperWidget = NodeWidget::makeNodeWidgetForCompositeNodeStealingContentWidgets(memberWidgets, node, var.name, widgetOptionsOpt);
            SV_ASSERT(finalWrapperWidget);
            WidgetsForNodeManager::registerWidgetForNode(node, finalWrapperWidget);
            *outWidget = finalWrapperWidget;
        }
        return node;
    }
}

bool SUP_TreeBuilder::addTabInformationIfNeeded(QJsonObjectWithWidgetOptionsOpt& outWidgetOptionsOpt, const SUP_ArglistOpt& arglistFromMacroString)
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