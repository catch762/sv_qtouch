#pragma once
#include "sv_qtcommon.h"
#include "DataNode/DataNodeHeader.h"
#include "SUP_Data/SUP_Data.h"
#include "SUP_Data/SUP_DataParser.h"
#include "SUP_Data/SUP_TreeBuilder.h"
#include "WidgetLogic/WidgetsForNodeManager.h"

class TreeUpdater
{
public:
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

        auto replaceOldNodeWithRemadeNode = [&](DataNodeShared&         oldNodeBeingReplaced, 
                                                DataNodeShared          reconstructedNode,
                                                const DataNodeShared&   newNodeReference)
        {
            //1. Delete old widget for existing node that we are replacing
            deleteWidgetForNode(oldNodeBeingReplaced);

            //2. Replace node
            oldNodeBeingReplaced = reconstructedNode;

            //3. Remake widget
            WidgetOptionsJsonOpt optionsToRemakeWidget = getValueOpt(newWidgetOptions, ConstDataNodeWeak(newNodeReference));
        };

        tryUpdateOldSubtreeFromNew(oldTree, newTree, replaceOldNodeWithRemadeNode);

        SV_LOG(std::format("Upgraded old tree:\n{}\n", oldTree->toString()));

        if (treesAreStructurallyEqual(*newTree.get(), *oldTree.get()))
        {
            SV_LOG("Upgrading oldTree to newTree succeeded - they are structurally equal now.");
            return true;
        }
        else
        {
            SV_ERROR("Upgrading oldTree to newTree failed - they are NOT structurally equal");
            return false;
        }
    }

    //*******************************************************************************************************
    //  Shortest impl would be:
    //      oldNodeBeingReplaced = reconstructedNode;
    // 
    //  But you probably also want to delete old widget, reconstruct it, etc.
    //  All of this shouldnt be concern of 'tryUpdateOldSubtreeFromNew', so i pass this NodeReplacer into it.
    //*******************************************************************************************************
    using NodeReplacer = std::function<void(DataNodeShared&         oldNodeBeingReplaced, 
                                            DataNodeShared          reconstructedNode,
                                            const DataNodeShared&   newNodeReference)>;

    // This function makes 'oldNode' subtree structurally equal to 'newNode' subtree,
    // doing as little change as possible. It applies itself to subnodes recursively.
    // 
    // Note:
    //      - 'oldNode' is REFERENCE to shared ptr! Because it might remake the node and drop old value.
    //      - 'oldNode' may be nullptr.
    //      - It will assert that arguments do have same name, because the point is we are updating same variable node
    //      - This also assumes there cant be 2 children with same name (as of now, there actually can be multiple in DataNode)
    static void tryUpdateOldSubtreeFromNew( DataNodeShared&         oldNode, 
                                            const DataNodeShared&   newNode, 
                                            const NodeReplacer&     nodeReplacer = TreeUpdater::basicNodeReplacer)
    {
        SV_ASSERT(newNode);

        // If oldNode is nullptr, its handled here
        if (oldNodeShouldClearlyBeCompletelyRemade(oldNode, newNode))
        {
            SV_LOG("upd", std::format("Remaking from the get go: {}", newNode->getName()));

            nodeReplacer(oldNode, DataNode::makeCopy(newNode), newNode);

            return;
        }

        // Now we are sure oldNode is not nullptr, and we can run some asserts:
        const bool oldIsLeaf = oldNode->isLeaf();
        const bool newIsLeaf = newNode->isLeaf();
        SV_ASSERT(oldNode);
        SV_ASSERT(oldNode->getName() == newNode->getName());
        SV_ASSERT(oldIsLeaf == newIsLeaf);

        if (oldIsLeaf)
        {
            //not handling leaves for now
            SV_LOG("upd", std::format("Skipping leaf update: {}", oldNode->getName()));
            return;
        }

        SV_LOG("upd", std::format("Gonna update children: {}", oldNode->getName()));

        std::vector<DataNodeShared> updatedChildrenListForOldNode;

        for (const DataNodeShared& newChild : newNode->tryGetCompositeData()->getChildren())
        {
            //may be null and its fine
            DataNodeShared oldChildWithSameName = oldNode->tryGetCompositeData()->getChild(newChild->getName());

            tryUpdateOldSubtreeFromNew(oldChildWithSameName, newChild, nodeReplacer);

            updatedChildrenListForOldNode.push_back(oldChildWithSameName);
        }

        oldNode->tryGetCompositeData()->setChildren(updatedChildrenListForOldNode, oldNode);
    }

private:
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

    static void deleteWidgetForNode(const DataNodeShared& node)
    {
        if (!node) return;

        if (auto widget = WidgetsForNodeManager::getSaveablePrimaryWidgetForNode(node))
        {
            widget->deleteLater();
        }
    }

    static void basicNodeReplacer(  DataNodeShared&         oldNodeBeingReplaced, 
                                    DataNodeShared          reconstructedNode,
                                    const DataNodeShared&   newNodeReference )
    {
        oldNodeBeingReplaced = reconstructedNode;
    }
};