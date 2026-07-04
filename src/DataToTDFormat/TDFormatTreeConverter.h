#pragma once
#include "TDFormatConverterInterface.h"
#include "DataNode/DataNodeHeader.h"

//result is passed to reuse memory, is cleared at start
StringErrOpt convertTreeToTDFormat(DataNodeShared tree, TreeAsTDFormatData &result);