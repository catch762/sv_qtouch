#include "TreeAndWidgetsBuilder.h"
#include "GLSLNativeTypes.h"
#include "SerializationLogic/SerializationSystem.h"

NodeAndWidgetPairOpt TreeAndWidgetsBuilder::buildTreeAndWidgetsForVariable(const SUP_Data &data, const SUP_Variable &var)
{
    if (auto typeId = GLSLNativeTypes::getTypeIndex(var.name))
    {
        if (auto serializerEntry = SerializationSystem::instance().getSerializerByIndex(*typeId))
        {
            QVariant defaultValueOfThisType = serializerEntry->defaultValueMaker();

            auto node = new DataNode(var.name, DataNode::NodeType::Leaf);
            *node->tryGetLeafvalue() = defaultValueOfThisType;


//okay json to both from ui................



        }
    }

    /*
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
    */
}
