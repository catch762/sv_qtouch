#pragma once
#include "TDFormatConverterInterface.h"
#include "DataNode/DataNodeHeader.h"

//result is passed to reuse memory, is cleared at start
StringErrOpt convertTreeToVec4Array(DataNodeShared tree, TreeAsVec4Array &result);

//result is passed to reuse memory, is cleared at start
StringErrOpt getVarNamesFromTree(DataNodeShared tree, TreeVarNames &result);

