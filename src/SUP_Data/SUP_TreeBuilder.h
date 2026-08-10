#pragma once
#include "SUP_Data/SUP_Data.h"
#include "DataNode/DataNode.h"
#include "WidgetLogic/WidgetDefs.h"
#include "WidgetLogic/NodeWidget.h"

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

class SUP_TreeBuilder
{
public:
    //There is guarantee that it either:
    //      - returns non-null DataNode (root) and as many widgets as root has children in 'outTopLevelWidgets';
    //      - returns nullptr and empty 'outTopLevelWidgets'
    //
    //So you dont need to cleanup anything in case of failure.
    static DataNodeShared buildTreeAndWidgets(const SUP_Data& data, NodeWidgetVec& outTopLevelWidgets);


    static DataNodeShared buildTree(const SUP_Data& data, MapOfWidgetOptionsForNodes* outWidgetOptions = nullptr);

private:
    // Optional parameters:
    //      -outTopLevelWidgets
    //          If you do supply it, then it will build a widget for each tree node as well,
    //          and return top level widgets in this parameter.
    //      -outWidgetOptions
    //          Will put every non-empty WidgetOptions in that map
    static DataNodeShared buildTreeAndOptionallyWidgets(const SUP_Data& data, NodeWidgetVec* outTopLevelWidgets = nullptr, MapOfWidgetOptionsForNodes* outWidgetOptions = nullptr);

    //If you do supply 'outWidget', then it will build a widget for variable as well.
    //If you do supply 'outWidgetOptions', it will put every non-empty WidgetOptions for the entire subtree of this variable
    static DataNodeShared buildTreeForVariable( const SUP_Data&             data,
                                                const SUP_Variable&         var,
                                                NodeWidgetUnique*           outWidget = nullptr,
                                                MapOfWidgetOptionsForNodes* outWidgetOptions = nullptr);

    static WidgetOptionsJsonOptOrError makeWidgetOptionsFromMacroArglist(const SUP_ArglistOpt& arglistFromMacroString);

    // If ui macro string contains information about tab, we are adding it to widget options. (if input widget options is 
    // nullopt, then we create it)
    //
    // Return value: 'success'
    static StringErrOpt addTabInformationIfNeeded(WidgetOptionsJsonOpt& outWidgetOptionsOpt, const SUP_ArglistOpt& arglistFromMacroString);
};