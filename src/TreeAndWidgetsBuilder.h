#pragma once
#include "SUP_Data/SUP_Data.h"
#include "DataNode/DataNode.h"
#include "WidgetLogic/WidgetDefs.h"

//****************************************************************
//
// This class takes SUP_Data (that we obtained from GLSL code)
// that represents information about all (declared in a special way)
// uniform variables of a GLSL shader.
//
// It then builds matching C++ entities: DataNode tree along with
// widgets for each node (so we get a widget for each variable).
//
// It returns a flat list of {DataNode, widget} pairs, that are not
// parented to anything, its other classes responsibility to store
// this data, add widgets to some layout, etc.
//
// Only top level items are returned! E.g. result will hold as many
// items as there are in data.varListEntries 
//
//****************************************************************

// 1. DataNodeShared is 'root', containing all actual sub nodes that were created
//    from code / deserializing process. Just so we unite them all in one node, and
//    not in vector of nodes. There is no widget for 'root'.
// 2. NodeWidgetVec is vector of widgets for all immediate children of 'root'
using TreeAndTopLevelWidgets = std::pair<DataNodeShared, NodeWidgetVec>;
SV_DECL_OPT(TreeAndTopLevelWidgets);

class TreeAndWidgetsBuilder
{
public:
    static TreeAndTopLevelWidgetsOpt buildTreeAndWidgets(const SUP_Data& data);

private:
    static NodeAndWidgetPairOpt buildTreeAndWidgetsForVariable(const SUP_Data& data, const SUP_Variable& var);
};