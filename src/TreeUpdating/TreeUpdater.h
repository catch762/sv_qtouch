#pragma once
#include "sv_qtcommon.h"
#include "DataNode/DataNodeHeader.h"
#include "SUP_Data/SUP_Data.h"
#include "SUP_Data/SUP_DataParser.h"
#include "SUP_Data/SUP_TreeBuilder.h"
#include "WidgetLogic/WidgetsForNodeManager.h"
#include "WidgetLogic/WidgetMakerSystem.h"

#include "UiComponents/TopLevelWidgetsContainer.h"


class TreeUpdateOperations
{
public:
    virtual void beforeReplacingNode(const DataNodeShared&   oldNodeBeingReplaced,
                                     const DataNodeShared&   newNodeReference,
                                     DataNodeShared&         replacementNode)
    {
    }

    virtual void beforeAddingCompletelyNewNode(DataNodeShared& nodeAdded,
                                               const DataNodeShared& newNodeReference)
    {
    }

    virtual void beforeRemovingNode(const DataNodeShared& nodeThatWillBeRemoved)
    {
    }
};

class TreeUpdateOperationsWithWidgetsUpdating : public TreeUpdateOperations
{
public:
    TreeUpdateOperationsWithWidgetsUpdating(MapOfWidgetOptionsForNodes&& _newWidgetOptions)
    {
        newWidgetOptions = std::move(_newWidgetOptions);
    }

    void beforeReplacingNode(const DataNodeShared& oldNodeBeingReplaced,
                             const DataNodeShared& newNodeReference,
                             DataNodeShared&       replacementNode) override
    {
        deleteWidgetForNode(oldNodeBeingReplaced);

        auto widget = createWidgetForNode(replacementNode, newNodeReference);
    }

    void beforeAddingCompletelyNewNode(DataNodeShared&       nodeAdded,
                                       const DataNodeShared& newNodeReference) override
    {
        auto widget = createWidgetForNode(nodeAdded, newNodeReference);
    }

    void beforeRemovingNode(const DataNodeShared& nodeThatWillBeRemoved) override
    {
        deleteWidgetForNode(nodeThatWillBeRemoved);
    }

private:
    static void deleteWidgetForNode(const DataNodeShared& node)
    {
        if (auto widget = WidgetsForNodeManager::getSaveablePrimaryWidgetForNode(node))
        {
            widget->deleteLater();
        }
    }

    NodeWidget* createWidgetForNode(const DataNodeShared& node, const DataNodeShared& newNodeReference)
    {
        WidgetOptionsJsonOpt optionsToRemakeWidget = getValueOpt(newWidgetOptions, ConstDataNodeWeak(newNodeReference));
        auto createdWidget = WidgetMakerSystem::instance().createAndRegisterWidgetForNode(node, optionsToRemakeWidget);
        SV_ASSERT(createdWidget);








        createdWidget->show();
        return createdWidget;
    }

private:
    MapOfWidgetOptionsForNodes newWidgetOptions;
};

class TreeUpdater
{
public:
    static bool updateTreeFromCode( DataNodeShared&             oldTree, 
                                    const QStringVec&           newCodeFilePaths, 
                                    TopLevelWidgetsContainer*   topLevelWidgetContainer = nullptr )
    {
        bool updatingWidgetsRequested = topLevelWidgetContainer;

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

        auto updateOperationsWithWidgets = std::make_unique<TreeUpdateOperationsWithWidgetsUpdating>(std::move(newWidgetOptions));

        tryUpdateOldSubtreeFromNew( oldTree,
                                    newTree,
                                    updatingWidgetsRequested ? updateOperationsWithWidgets.get() : nullptr);

        SV_LOG(std::format("Upgraded old tree:\n{}\n", oldTree->toString()));

        if (!treesAreStructurallyEqual(*newTree.get(), *oldTree.get()))
        {
            SV_ERROR("Upgrading oldTree to newTree failed - they are NOT structurally equal");
            return false;
        }

        if (updatingWidgetsRequested)
        {
            NodeWidgetQPointerVec topLevelWidgets;
            SV_ASSERT(oldTree);
            SV_ASSERT(oldTree->isComposite());

            for (const auto& topLevelChild : oldTree->tryGetCompositeData()->getChildren())
            {
                auto topLevelWidget = WidgetsForNodeManager::getSaveablePrimaryWidgetForNode(topLevelChild);
                if (!topLevelWidget)
                {
                    SV_ERROR(std::format("Upgrading oldTree to newTree failed, oldTree itself was updated and is equal to "
                                         "newTree, but its immediate child [{}] doesnt have widget associated with it!",
                                         topLevelChild));

                    //this is serious, perhaps we should delete all widgets now?

                    return false;
                }

                topLevelWidgets.push_back(topLevelWidget);
            }

            // Some of previously existing widgets are already deleted, but some of existing widgets might have persisted,
            // and are now in 'topLevelWidgets'. So we are not deleting any.
            topLevelWidgetContainer->setTopLevelWidgets(std::move(topLevelWidgets), 
                TopLevelWidgetsContainer::ExistingWidgetsPolicy::UnparentButKeepAlive);
        }

        SV_LOG("Upgrading oldTree to newTree succeeded - they are structurally equal now.");

        return true;
    }



    // This function makes 'oldNode' subtree structurally equal to 'newNode' subtree,
    // doing as little change as possible. It applies itself to subnodes recursively.
    // 
    // Note:
    //      - 'oldNode' is REFERENCE to shared ptr! Because it might remake the node and drop old value.
    //      - 'oldNode' may be nullptr.
    //      - It will assert that arguments do have same name, because the point is we are updating same variable node
    //      - This also assumes there cant be 2 children with same name (as of now, there actually can be multiple in DataNode)
    static DataNodeShared tryUpdateOldSubtreeFromNew( DataNodeShared&         oldNode,
                                                      const DataNodeShared&   newNode, 
                                                      TreeUpdateOperations*   operations = nullptr )
    {
        SV_ASSERT(newNode);

        // If oldNode is nullptr, its handled here
        if (oldNodeShouldClearlyBeCompletelyRemade(oldNode, newNode))
        {
            auto reconstructedNode = DataNode::makeCopy(newNode);
            if (oldNode)
            {
                SV_LOG("upd", std::format("Replacing node: {}", newNode->getName()));
                if (operations) operations->beforeReplacingNode(oldNode, newNode, reconstructedNode);
            }
            else
            {
                SV_LOG("upd", std::format("Creating comepletely new node: {}", newNode->getName()));
                if (operations) operations->beforeAddingCompletelyNewNode(reconstructedNode, newNode);
            }
            return reconstructedNode;
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
            return oldNode;
        }

        SV_LOG("upd", std::format("> Gonna update children: {}", oldNode->getName()));

        std::vector<DataNodeShared> updatedChildrenListForOldNode;

        //So we can track which child in old node we need to delete afterwards
        std::vector<bool> oldChildrenStillExistsRegistry(oldNode->childrenCount(), false);

        for (const DataNodeShared& newChild : newNode->tryGetCompositeData()->getChildren())
        {
            //May not exist, and its fine
            int             oldChildWithSameNameIndex = -1;
            DataNodeShared  oldChildWithSameName      = oldNode->tryGetCompositeData()->getChild(newChild->getName(), &oldChildWithSameNameIndex);

            DataNodeShared  updatedChild              = tryUpdateOldSubtreeFromNew(oldChildWithSameName, newChild, operations);

            updatedChildrenListForOldNode.push_back(updatedChild);

            if (oldChildWithSameNameIndex != -1)
            {
                SV_ASSERT(isValidIndex(oldChildWithSameNameIndex, oldChildrenStillExistsRegistry.size()));

                oldChildrenStillExistsRegistry[oldChildWithSameNameIndex] = true;
            }
        }

        // Handling children of oldNode that will be deleted: 
        for (int oldChildIndex = 0; oldChildIndex < oldNode->childrenCount(); ++oldChildIndex)
        {
            SV_ASSERT(isValidIndex(oldChildIndex, oldChildrenStillExistsRegistry.size()));

            bool oldChildWillBeDeletedNow = !oldChildrenStillExistsRegistry[oldChildIndex];
            if (oldChildWillBeDeletedNow)
            {
                auto oldChildToBeDeleted = oldNode->tryGetChild(oldChildIndex);

                SV_LOG("upd", std::format("Deleting child node: {}", oldChildToBeDeleted->getName()));
                if (operations) operations->beforeRemovingNode(oldChildToBeDeleted);
            }
        }

        // The line that actually removes those 'to be deleted' children is just this one.
        // (assuming nothing else holds their shared pointers, these children will go out of scope now)
        oldNode->tryGetCompositeData()->setChildren(updatedChildrenListForOldNode, oldNode);

        return oldNode;
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
};