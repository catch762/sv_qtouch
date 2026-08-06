#include "sv_qtcommon.h"
#include "DataNode/DataNodeHeader.h"
#include "SUP_Data/SUP_Data.h"

class TreeUpdater
{
public:
	void updateTree(DataNodeShared tree, const SUP_Data& newData);
};