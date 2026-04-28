#pragma once
#include "SUP_Data/SUP_Data.h"
#include "DataNode/DataNode.h"
#include "WidgetLogic/WidgetDefs.h"

//****************************************************************
//
// This class takes SUP_Data (that we obtained from GLSL code)
// that represents GLSL project uniform variables.
//
// It then builds matching C++ entities: DataNode tree along with
// widgets for each node (so we get a widget for each variable).
//
//****************************************************************
class TreeAndWidgetsBuilder
{
public:

    //returns root
    NodeAndWidgetPairOpt buildTreeAndWidgets(const SUP_Data& data)
    {
        auto rootNode = DataNode::makeComposite("root");

        int entryIdx = 0;
        for (auto &varlistEntry : data.varListEntries)
        {
            auto nodeAndWidget = buildTreeAndWidgetsForVarListEntry(data, varlistEntry);
            if (!nodeAndWidget)
            {
                SV_ERROR(std::format("Failed building NodeAndWidgetPair for entry [{}], {}", entryIdx, varlistEntry));
                return {};
            }
            if (!nodeAndWidget->isValid())
            {
                SV_ERROR(std::format("Obtained invalid NodeAndWidgetPair for entry [{}], {}", entryIdx, varlistEntry));
                return {};
            }

            rootNode->addChild(nodeAndWidget->node);

            entryIdx++;
        }

        return NodeAndWidgetPair{rootNode};
    }

private:
    NodeAndWidgetPairOpt buildTreeAndWidgetsForVarListEntry(const SUP_Data& data, const SUP_VarListEntry& entry)
    {
        if (entry.macroType == SUP_VarListEntry::MacroType::ScalarVariable)
        {
            return {};
        }
        else if (entry.macroType == SUP_VarListEntry::MacroType::Struct)
        {
            return buildTreeAndWidgetsForStruct(data, entry);
        }
        else SV_UNREACHABLE();
    }

    NodeAndWidgetPairOpt buildTreeAndWidgetsForStruct(const SUP_Data& data, const SUP_VarListEntry& structEntry)
    {
        SV_ASSERT(structEntry.macroType == SUP_VarListEntry::MacroType::Struct);

        auto* structDef = data.getStruct(structEntry.varType);
        if (!structDef)
        {
            SV_ERROR(std::format("TreeAndWidgetsBuilder: VarList had structure type [{}], but no such struct definition",
                                    structEntry.varType));
            return {};
        }

        for (const SUP_StructMember& member : structDef->members)
        {
            
        }
    }
};