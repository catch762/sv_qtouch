#include "sv_qtcommon.h"
#include "DataNode/DataNodeHeader.h"
#include "SUP_Data/SUP_Data.h"

class TreeUpdater
{
public:
    static void deleteWidgetForNode(const DataNodeShared& node)
    {
        if (!node) return;

        if (auto widget = WidgetsForNodeManager::getSaveablePrimaryWidgetForNode(node))
        {
            widget->deleteLater();
        }
    }

    static bool updateTreeFromCode(DataNodeShared& oldTree, const QStringVec& newCodeFilePaths)
    {
        SV_LOG(std::format("Updating from new code: {}", newCodeFilePaths));

        auto showErr = [](std::string err)
        {
            SV_MSGBOX_ERROR(std::format("updateTreeFromCode failed: {}", err));
        };

        if (!oldTree)
        {
            showErr("old tree is null");
            return false;
        }

        auto newVarData = SUP_DataParser().parseFiles(newCodeFilePaths);
        if (!newVarData)
        {
            showErr("failed to parse new GLSL code.");
            return false;
        }

        MapOfWidgetOptionsForNodes newWidgetOptions;
        auto newTree = SUP_TreeBuilder::buildTree(*newVarData, &newWidgetOptions);
        if (!newTree)
        {
            showErr("couldnt build new tree from parsed new GLSL code");
            return false;
        }

        SV_LOG(std::format("New tree:\n{}\n", newTree->toString()));
        SV_LOG(std::format("Old tree:\n{}\n", oldTree->toString()));

        tryUpdateOldSubtreeFromNew(oldTree, newTree);

        SV_LOG(std::format("Upgraded old tree:\n{}\n", oldTree->toString()));

        if (treesAreStructurallyEqual(*newTree.get(), *oldTree.get()))
        {
            SV_LOG("Upgrading oldTree to newTree succeeded - they are structurally equal now.");
        }
        else
        {
            SV_ERROR("Upgrading oldTree to newTree failed - they are NOT structurally equal");
        }
    }

    // This function makes 'oldNode' subtree structurally equal to 'newNode' subtree,
    // doing as little change as possible. It applies itself to subnodes recursively.
    // 
    // Note:
    //      - 'oldNode' is REFERENCE to shared ptr! Because it might remake the node and drop old value.
    //      - It will assert that arguments do have same name, because the point is we are updating same variable node
    //      - This also assumes there cant be 2 children with same name (as of now, there actually can be multiple in DataNode)
    static void tryUpdateOldSubtreeFromNew(DataNodeShared& oldNode, const DataNodeShared& newNode)
    {
        SV_ASSERT(oldNode);
        SV_ASSERT(newNode);
        SV_ASSERT(oldNode->getName() == newNode->getName());

        const bool oldIsLeaf = oldNode->isLeaf();
        const bool newIsLeaf = newNode->isLeaf();

        if (oldNodeShouldClearlyBeCompletelyRemade(oldNode, newNode))
        {
            SV_LOG("upd", std::format("Should be remade from the get go: {}", oldNode->getName()));

            deleteWidgetForNode(oldNode);
            oldNode = DataNode::makeCopy(newNode);

            return;
        }

        SV_ASSERT(oldIsLeaf == newIsLeaf);

        if (oldIsLeaf)
        {
            //not handling leaves for now
            SV_LOG("upd", std::format("Skipping leaf update: {}", oldNode->getName()));
            return;
        }

        std::vector<DataNodeShared> updatedChildrenListForOldNode;

        for (const DataNodeShared& newChild : newNode->tryGetCompositeData()->getChildren())
        {
            DataNodeShared oldChildWithSameName = oldNode->tryGetCompositeData()->getChild(newChild->getName());

            const bool shouldRemake = oldNodeShouldClearlyBeCompletelyRemade(oldChildWithSameName, newChild);

            if (shouldRemake)
            {
                SV_LOG("upd", std::format("Updating {} --> remaking child {}", oldNode->getName(), newChild->getName()));

                deleteWidgetForNode(oldChildWithSameName);
                DataNodeShared remadeChild = DataNode::makeCopy(newChild);
                updatedChildrenListForOldNode.push_back(remadeChild);
            }
            else
            {
                SV_ASSERT(oldChildWithSameName);

                SV_LOG("upd", std::format("Updating {} --> updating child subtree {}", oldNode->getName(), newChild->getName()));

                tryUpdateOldSubtreeFromNew(oldChildWithSameName, newChild);

                updatedChildrenListForOldNode.push_back(oldChildWithSameName);
            }
        }

        oldNode->tryGetCompositeData()->setChildren(updatedChildrenListForOldNode, oldNode);
    }

    // This function checks if these two nodes are clearly very different (and thus it doesnt make sense
    // updating the old node, we are gonna just remake it from scratch).
    // 
    // It will ASSERT that arguments do have same name, because the point is
    // we are updating same variable in two trees. 
    static bool oldNodeShouldClearlyBeCompletelyRemade(const DataNodeShared& oldNode, const DataNodeShared& newNode)
    {
        if (!oldNode)
        {
            return true;
        }

        SV_ASSERT(newNode);
        SV_ASSERT(oldNode->getName() == newNode->getName());

        const bool oldIsLeaf = oldNode->isLeaf();
        const bool newIsLeaf = newNode->isLeaf();

        // comp/leaf mismatch
        if (oldIsLeaf != newIsLeaf)
        {
            return true;
        }

        // leaf type mismatch
        if (oldIsLeaf && newIsLeaf && !anyHoldSameType(oldNode->tryGetLeafvalue(), newNode->tryGetLeafvalue()))
        {
            return true;
        }

        // everything else is fine
        return false;
    }
};