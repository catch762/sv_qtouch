#include "SUP_TreeBuilder.h"
#include "SUP_Data/SUP_NativeGLSLTypeConverter.h"
#include "SerializationLogic/SerializationSystem.h"
#include "WidgetLogic/WidgetMakerSystem.h"
#include "WidgetLogic/NodeWidget.h"
#include "WidgetLogic/WidgetsForNodeManager.h"
#include "QTouchDefs.h"

bool allOrNoneRequiredResourcesAcquired(   const DataNodeShared&   resource0_tree,
                                           const NodeWidgetUnique& resource1_widget,
                                           bool                    widgetWasRequested )
{
    if (widgetWasRequested)
    {
        bool haveBoth = resource0_tree && resource1_widget;
        bool haveNone = !resource0_tree && !resource1_widget;
        return haveBoth || haveNone;
    }
    else
    {
        return true;
    }
}

DataNodeShared SUP_TreeBuilder::buildTree(const SUP_Data& data, MapOfWidgetOptionsForNodes* outWidgetOptions)
{
    auto result = buildTreeAndOptionallyWidgets(data, nullptr, outWidgetOptions);

    if (!result && outWidgetOptions) outWidgetOptions->clear();

    return result;
}

DataNodeShared SUP_TreeBuilder::buildTreeAndWidgets(const SUP_Data& data, NodeWidgetQPointerVec& outTopLevelWidgets)
{
    return buildTreeAndOptionallyWidgets(data, &outTopLevelWidgets);
}

DataNodeShared SUP_TreeBuilder::buildTreeAndOptionallyWidgets(  const SUP_Data&             data, 
                                                                NodeWidgetQPointerVec*      outTopLevelWidgets, 
                                                                MapOfWidgetOptionsForNodes* outWidgetOptions )
{
    const bool widgetsRequested = static_cast<bool>(outTopLevelWidgets);

    std::vector<DataNodeShared>     topLevelNodes;
    std::vector<NodeWidgetUnique>   topLevelWidgets;

    int entryIdx = 0;
    for (auto &varlistEntry : data.varListEntries)
    {
        NodeWidgetUnique optionalWidget;
        auto             node = buildTreeForVariable(data,
                                                     varlistEntry, 
                                                     widgetsRequested ? &optionalWidget : nullptr,
                                                     outWidgetOptions);

        SV_ASSERT(allOrNoneRequiredResourcesAcquired(node, optionalWidget, widgetsRequested));

        if (!node)
        {
            SV_ERROR(std::format("Failed building valid NodeAndWidgetPairList for entry [{}], {}", entryIdx, varlistEntry));
            return {};
        }

        topLevelNodes.push_back(std::move(node));
        if (widgetsRequested)
        {
            topLevelWidgets.push_back(std::move(optionalWidget));
        }

        entryIdx++;
    }

    if (topLevelNodes.empty())
    {
        SV_WARN("SUP_TreeBuilder finishing operation with 0 items created");
    }

    DataNodeShared root = DataNode::makeComposite();

    for(auto &node : topLevelNodes)
    {
        root->addChild(node);
    }

    if (widgetsRequested)
    {
        outTopLevelWidgets->clear();
        for (auto& widgetUnique : topLevelWidgets)
        {
            outTopLevelWidgets->push_back(widgetUnique.release());
        }
    }

    return std::move(root);
}

DataNodeShared SUP_TreeBuilder::buildTreeForVariable( const SUP_Data&             data,
                                                      const SUP_Variable&         var,
                                                      NodeWidgetUnique*           outWidget,
                                                      MapOfWidgetOptionsForNodes* outWidgetOptions)
{
    // Note: if im creating widgets, and then encounter error and return --
    // raw Widget* ptrs would remain in memory. So UNTIL i do return successfully,
    // widgets are wrapped in unique ptrs.

    const bool widgetsRequested         = static_cast<bool>(outWidget);
    const bool widgetOptionsRequested   = static_cast<bool>(outWidgetOptions);

    auto widgetOptionsOptOrErr = makeWidgetOptionsFromMacroArglist(var.arglistFromUiMacro);
    if (auto err = getError(widgetOptionsOptOrErr))
    {
        SV_ERROR(std::format("buildTreeForVariable: failed to make widget options, error: {}", *err));
        return {};
    }

    auto saveWidgetOptionsIfNeeded = [&](ConstDataNodeWeak nodeKey)
    {
        auto widgetOptionsOpt = getValue(widgetOptionsOptOrErr);

        if (widgetOptionsRequested && widgetOptionsOpt->has_value())
        {
            (*outWidgetOptions)[nodeKey] = std::move(widgetOptionsOpt->value());
        }
    };

    if (SUP_NativeGLSLTypeConverter::instance().hasConverterForType(var.type)) //THEN ITS A NATIVE TYPE SO WE JUST MAKE IT
    {
        anyOpt res = SUP_NativeGLSLTypeConverter::instance().convert(var.type, var.arglistFromUiMacro);
        if (!res)
        {
            SV_ERROR(std::format("buildTreeForVariable: failed to convert seemingly native {} to C++ value", var));
            return {};
        }

        auto node = DataNode::makeLeaf(var.name, *res);
        SV_ASSERT(node);

        if (widgetsRequested)
        {
            outWidget->reset( WidgetMakerSystem::instance().createAndRegisterWidgetForNode(node, *getValue(widgetOptionsOptOrErr)) );
            if (!outWidget)
            {
                SV_ERROR(std::format("buildTreeForVariable: failed to make widget for {} representing seemingly native {}", node, var));
                return {};
            }
        }

        
        saveWidgetOptionsIfNeeded(node);
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

        std::vector<NodeWidgetUnique> memberWidgets;

        for (const auto& member : structDefinition->members)
        {
            NodeWidgetUnique    optionalMemberWidget;
            auto                memberNode           = buildTreeForVariable(data,
                                                                            member, 
                                                                            widgetsRequested ? &optionalMemberWidget : nullptr,
                                                                            outWidgetOptions);

            SV_ASSERT(allOrNoneRequiredResourcesAcquired(memberNode, optionalMemberWidget, widgetsRequested));

            if (!memberNode)
            {
                SV_ERROR(std::format("buildTreeForVariable: failed making valid member {} of struct {}", member, *structDefinition));
                return {};
            }

            node->addChild(memberNode);

            if (widgetsRequested)
            {
                memberWidgets.push_back(std::move(optionalMemberWidget));
            }
        }

        if (widgetsRequested)
        {
            //this cant fail
            outWidget->reset(NodeWidget::makeNodeWidgetForCompositeNodeStealingContentWidgets(memberWidgets, node, var.name, *getValue(widgetOptionsOptOrErr)));
            SV_ASSERT(*outWidget);
            WidgetsForNodeManager::registerWidgetForNode(node, outWidget->get());
        }

        saveWidgetOptionsIfNeeded(node);
        return node;
    }
}

WidgetOptionsJsonOptOrError SUP_TreeBuilder::makeWidgetOptionsFromMacroArglist(const SUP_ArglistOpt& arglistFromMacroString)
{
    WidgetOptionsJsonOpt widgetOptionsOpt = {};

    if (auto err = addTabInformationIfNeeded(widgetOptionsOpt, arglistFromMacroString))
    {
        return *err;
    }

    return widgetOptionsOpt;
}

StringErrOpt SUP_TreeBuilder::addTabInformationIfNeeded(WidgetOptionsJsonOpt& outWidgetOptionsOpt, const SUP_ArglistOpt& arglistFromMacroString)
{
    if (arglistFromMacroString)
    {
        if (const SUP_Expr* tabArg = arglistFromMacroString->getArgByName("tab"))
        {
            if (!tabArg->isLeaf() || !tabArg->getLeafValue()->isNumberInt())
            {
                return std::format("addTabInformationIfNeeded failed: tabArg isnt NumberInt leaf: {}", *tabArg);
            }

            TabIndex tabIndex = tabArg->getLeafValue()->getNumberIntData();

            //we dont need to save default val
            if (tabIndex == 1)
            {
                return {};
            }

            if (!outWidgetOptionsOpt)
            {
                outWidgetOptionsOpt = WidgetOptionsJson();
            }

            (*outWidgetOptionsOpt)[NodeWidget::tabIndexKey] = tabIndex;
        }
    }

    return {};
}