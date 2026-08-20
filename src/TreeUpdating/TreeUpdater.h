#pragma once
#include "sv_qtcommon.h"
#include "DataNode/DataNodeHeader.h"
#include "SUP_Data/SUP_Data.h"
#include "SUP_Data/SUP_DataParser.h"
#include "SUP_Data/SUP_TreeBuilder.h"
#include "WidgetLogic/WidgetsForNodeManager.h"
#include "WidgetLogic/WidgetMakerSystem.h"

#include "UiComponents/TopLevelWidgetsContainer.h"

//This struct represents parent of some other child node; it is used in context
//of algorithm which needs to operate on (future) parent before its
//actually set on the child node. So it cant just get it via 'childNode->getParent()'.
struct ParentingData
{
    DataNodeShared  parentNode;
    int             indexInParent = -1;
};
SV_DECL_OPT(ParentingData);

class TreeUpdateOperations
{
public:
    virtual void beforeReplacingNode(const DataNodeShared&   oldNodeBeingReplaced,
                                     const DataNodeShared&   newNodeReference,
                                     const DataNodeShared&   replacementNode,
                                     ParentingDataOpt        replacementParenting)
    {
    }

    virtual void beforeAddingCompletelyNewNode(const DataNodeShared& nodeAdded,
                                               ParentingDataOpt      nodeParenting,
                                               const DataNodeShared& newNodeReference)
    {
    }

    virtual void beforeRemovingNode(const DataNodeShared& nodeThatWillBeRemoved)
    {
    }

    virtual WidgetOptionsJsonOpt getOptionsForOldNode(ConstDataNodeWeak oldNode) = 0;
    virtual WidgetOptionsJsonOpt getOptionsForNewNode(ConstDataNodeWeak newNode) = 0;

public:
    bool nodesHaveSameCreationStringInOptions(ConstDataNodeWeak oldNode, ConstDataNodeWeak newNode)
    {
        QStringOpt oldCreationString = getCreationStringOpt(getOptionsForOldNode(oldNode));
        QStringOpt newCreationString = getCreationStringOpt(getOptionsForNewNode(newNode));

        const auto hasOld = bool(oldCreationString);
        const auto hasNew = bool(newCreationString);

        if (hasOld == hasNew)
        {
            if (hasOld && hasNew)
            {
                return *oldCreationString == *newCreationString;
            }
            else
            {
                return true;
            }
        }
        else
        {
            return false;
        }
    }

    virtual ~TreeUpdateOperations() = default;
};

class UpdateOperationsForPlainTreeWithoutWidgets : public TreeUpdateOperations
{
public:
    UpdateOperationsForPlainTreeWithoutWidgets(MapOfWidgetOptionsForNodes& _oldWidgetOptions,
                                               const MapOfWidgetOptionsForNodes& _newWidgetOptions)
        :   oldWidgetOptions(_oldWidgetOptions), 
            newWidgetOptions(_newWidgetOptions)
    {
    }

    void beforeReplacingNode(const DataNodeShared& oldNodeBeingReplaced,
                             const DataNodeShared& newNodeReference,
                             const DataNodeShared& replacementNode,
                             ParentingDataOpt      replacementParenting) override
    {
        saveOptionsForAllNodesInFreshlyMadeTree(replacementNode, newNodeReference);
    }

    void beforeAddingCompletelyNewNode(const DataNodeShared& nodeAdded,
                                       ParentingDataOpt      nodeParenting,
                                       const DataNodeShared& newNodeReference) override
    {
        saveOptionsForAllNodesInFreshlyMadeTree(nodeAdded, newNodeReference);
    }

    WidgetOptionsJsonOpt getOptionsForOldNode(ConstDataNodeWeak oldNode) override
    {
        return getValueOpt(oldWidgetOptions, oldNode);
    }
    WidgetOptionsJsonOpt getOptionsForNewNode(ConstDataNodeWeak newNode) override
    {
        return getValueOpt(newWidgetOptions, newNode);
    }

    StringErrOpt getFirstError()
    {
        return firstError;
    }

private:
    // we might have copied subtree from newTree, and then (for each subnode, possibly) we have to pull 
    // widget options from that newTree widget options map
    void saveOptionsForAllNodesInFreshlyMadeTree(const DataNodeShared& nodeAdded, const DataNodeShared& newNodeReference)
    {
        SV_ASSERT(nodeAdded);
        SV_ASSERT(newNodeReference);

        auto mismatchInfo = visitTwoStructurallyEqualTrees_withMismatchInfo(*nodeAdded, *newNodeReference,
            [&](const DataNode& nodeAddedToOldTree, const DataNode& newNodeReference)
            {
                ConstDataNodeWeak nodeAddedToOldTreeWeak = nodeAddedToOldTree.weak_from_this();
                ConstDataNodeWeak newNodeReferenceWeak   = newNodeReference.weak_from_this();

                if (const WidgetOptionsJson* widgetOptions = getValue(newWidgetOptions, newNodeReferenceWeak))
                {
                    oldWidgetOptions[nodeAddedToOldTreeWeak] = *widgetOptions;
                }
            });

        if (mismatchInfo)
        {
            firstError = std::format("saveOptionsOfAllNodesInFreshlyMadeTree: mismatch between freshly made subtree and reference subtree: {}",
                                     *mismatchInfo);
        }
    }

private:
    MapOfWidgetOptionsForNodes&         oldWidgetOptions;
    const MapOfWidgetOptionsForNodes&   newWidgetOptions;

    StringErrOpt                        firstError;
};

class UpdateOperationsForLiveTreeWithWidgets : public TreeUpdateOperations
{
public:
    UpdateOperationsForLiveTreeWithWidgets(const MapOfWidgetOptionsForNodes& _newWidgetOptions)
        : newWidgetOptions(_newWidgetOptions)
    {
    }

    void beforeReplacingNode(const DataNodeShared& oldNodeBeingReplaced,
                             const DataNodeShared& newNodeReference,
                             const DataNodeShared& replacementNode,
                             ParentingDataOpt      replacementParenting) override
    {
        deleteWidgetForNode(oldNodeBeingReplaced);

        auto widget = createWidgetForNode(replacementNode, replacementParenting, UpdateStatus::RemadeVariable, newNodeReference);
    }

    void beforeAddingCompletelyNewNode(const DataNodeShared& nodeAdded,
                                       ParentingDataOpt      nodeParenting,
                                       const DataNodeShared& newNodeReference) override
    {
        auto widget = createWidgetForNode(nodeAdded, nodeParenting, UpdateStatus::NewlyMadeVariable, newNodeReference);
    }

    void beforeRemovingNode(const DataNodeShared& nodeThatWillBeRemoved) override
    {
        deleteWidgetForNode(nodeThatWillBeRemoved);
    }

    WidgetOptionsJsonOpt getOptionsForOldNode(ConstDataNodeWeak oldNode) override
    {
        if (auto widget = WidgetsForNodeManager::getSaveablePrimaryWidgetForNode(oldNode))
        {
            return widget->makeOptions();
        }
        else return {};
    }
    WidgetOptionsJsonOpt getOptionsForNewNode(ConstDataNodeWeak newNode) override
    {
        return getValueOpt(newWidgetOptions, newNode);
    }

private:
    static void deleteWidgetForNode(const DataNodeShared& node)
    {
        if (auto widget = WidgetsForNodeManager::getSaveablePrimaryWidgetForNode(node))
        {
            widget->deleteLater();
        }
    }

    NodeWidget* createWidgetForNode(const DataNodeShared& node,
                                    ParentingDataOpt      nodeParenting,
                                    UpdateStatus          nodeUpdateStatus,
                                    const DataNodeShared& newNodeReference)
    {
        WidgetOptionsJsonOpt optionsToRemakeWidget = getValueOpt(newWidgetOptions, ConstDataNodeWeak(newNodeReference));
        auto createdWidget = WidgetMakerSystem::instance().createAndRegisterWidgetForNode(node,
                                                                                          optionsToRemakeWidget,
                                                                                          &newWidgetOptions);
        SV_ASSERT(createdWidget);

        createdWidget->setUpdateStatus(nodeUpdateStatus);

        if (nodeParenting)
        {
            //ParentWidget will not exist for top level widgets. But if it exists, we need to add this widget to it
            if (auto parentWidget = WidgetsForNodeManager::getSaveablePrimaryWidgetForNode(nodeParenting->parentNode))
            {
                parentWidget->addContentWidget(createdWidget, nodeParenting->indexInParent);
            }
        }

        //createdWidget->show();
        return createdWidget;
    }

private:
    const MapOfWidgetOptionsForNodes& newWidgetOptions;
};

//This exists because there are 2 cases: obtaining options from parsed json and from a live widget
//using WidgetOptionsGetter = std::function<WidgetOptionsJsonOpt(ConstDataNodeWeak)>;

class TreeUpdater
{
public:
    static StringErrOpt updateTreeAndWidgetsFromCode(   DataNodeShared&             oldTree, 
                                                        const QStringVec&           newCodeFilePaths, 
                                                        TopLevelWidgetsContainer&   topLevelWidgetContainer)
    {
        auto wrapErr = [](const std::string &err)
        {
            return std::format("updateTreeAndWidgetsFromCode failed: {}", err);
        };

        if (!oldTree)
        {
            return wrapErr("old tree is null");
        }

        auto newVarData = SUP_DataParser().parseFiles(newCodeFilePaths);
        if (!newVarData)
        {
            return wrapErr("failed to parse new GLSL code.");
        }

        MapOfWidgetOptionsForNodes newWidgetOptions;
        auto newTree = SUP_TreeBuilder::buildTree(*newVarData, &newWidgetOptions);
        if (!newTree)
        {
            return wrapErr("couldnt build new tree from parsed new GLSL code");
        }

        auto updateErrOpt = updateTreeAndWidgetsFromCode(oldTree, newTree, newWidgetOptions, topLevelWidgetContainer);
        if (updateErrOpt)
        {
            return wrapErr(*updateErrOpt);
        }

        return {};
    }

    static StringErrOpt updatePreset(   const QString&                      oldPresetJsonAbsPath,
                                        const DataNodeShared&               newTree,
                                        const MapOfWidgetOptionsForNodes&   newWidgetOptions )
    {
        auto wrapErr = [&](const std::string& err)
        {
            return std::format("updatePreset from file=[{}] failed: {}", oldPresetJsonAbsPath, err);
        };

        auto oldPresetJson = loadJsonFromFile(oldPresetJsonAbsPath);
        if (!oldPresetJson)
        {
            return wrapErr("couldnt load json from file");
        }

        auto [oldPresetTree, oldPresetWidgetOptions] = SerializerForDataNodeTreeAndItsWidgets::jsonToRootNodeAndWidgetOptions(*oldPresetJson);
        if (!oldPresetTree)
        {
            return wrapErr("couldnt load tree from json");
        }

        //note: this is gonna update oldPresetWidgetOptions
        auto operations = std::make_unique<UpdateOperationsForPlainTreeWithoutWidgets>(std::ref(oldPresetWidgetOptions), 
                                                                                       std::cref(newWidgetOptions));

        oldPresetTree = tryUpdateOldSubtreeFromNew(oldPresetTree, newTree, {}, *operations);
        SV_ASSERT(oldPresetTree);

        if (auto firstErrFromOperations = operations->getFirstError())
        {
            return wrapErr(*firstErrFromOperations);
        }

        auto updatedTreeJson = SerializerForDataNodeTreeAndItsWidgets::toJson(oldPresetTree, oldPresetWidgetOptions);
        if (!updatedTreeJson)
        {
            return wrapErr("couldnt serialize updated tree back to json");
        }

        bool saved = saveJsonValueToFile(*updatedTreeJson, oldPresetJsonAbsPath);
        if (!saved)
        {
            return wrapErr("couldnt save updated tree json to file");
        }

        return {};
    }


private:
    static StringErrOpt updateTreeAndWidgetsFromCode(   DataNodeShared&                     oldTree,
                                                        const DataNodeShared&               newTree,
                                                        const MapOfWidgetOptionsForNodes&   newWidgetOptions,
                                                        TopLevelWidgetsContainer&           topLevelWidgetContainer )
    {
        SV_LOG(std::format("\nNew tree:\n{}\n", newTree->toString()));
        SV_LOG(std::format("Old tree:\n{}\n", oldTree->toString()));

        auto updateOperations = std::make_unique<UpdateOperationsForLiveTreeWithWidgets>(std::cref(newWidgetOptions));
        auto updateErrOpt     = updateTree(oldTree, newTree, *updateOperations);

        SV_LOG(std::format("Updated old tree:\n{}\n", oldTree->toString()));

        if (updateErrOpt)
        {
            return *updateErrOpt;
        }

        auto widgetUpdateErrOpt = setUpdatedTopLevelWidgetsOfTree(oldTree, topLevelWidgetContainer);
        if (widgetUpdateErrOpt)
        {
            return *widgetUpdateErrOpt;
        }

        return {};
    }

    static StringErrOpt setUpdatedTopLevelWidgetsOfTree(const DataNodeShared&     updatedOldTree, 
                                                        TopLevelWidgetsContainer& topLevelWidgetContainer)
    {
        NodeWidgetQPointerVec topLevelWidgets;
        SV_ASSERT(updatedOldTree);
        SV_ASSERT(updatedOldTree->isComposite());

        for (const auto& topLevelChild : updatedOldTree->tryGetCompositeData()->getChildren())
        {
            auto topLevelWidget = WidgetsForNodeManager::getSaveablePrimaryWidgetForNode(topLevelChild);
            if (!topLevelWidget)
            {
                return std::format("Updated tree's immediate child {} doesnt have widget associated with it, so cant "
                                   "set new widgets to TopLevelWidgetsContainer", topLevelChild);
            }

            topLevelWidgets.push_back(topLevelWidget);
        }

        // Some of previously existing widgets are already deleted, but some of existing widgets might have persisted,
        // and are now in 'topLevelWidgets'. So we are not deleting any.
        topLevelWidgetContainer.setTopLevelWidgets(std::move(topLevelWidgets),
                                                   TopLevelWidgetsContainer::ExistingWidgetsPolicy::UnparentButKeepAlive);

        return {};
    }

    static StringErrOpt updateTree( DataNodeShared&       oldTree, 
                                    const DataNodeShared& newTree, 
                                    TreeUpdateOperations& updateOperations )
    {
        oldTree = tryUpdateOldSubtreeFromNew(oldTree,
                                             newTree,
                                             {},
                                             updateOperations);

        if (auto mismatchErrOpt = treesAreStructurallyEqual_withMismatchInfo(*newTree.get(), *oldTree.get()))
        {
            return std::format("updating tree failed, result not structurally equal to reference:\n{}", *mismatchErrOpt);
        }

        return {};
    }

    // WARNING !!! CORRECT USAGE IS: 
    //      oldNode = tryUpdateOldSubtreeFromNew(oldNode, newNode, ...);
    //
    // This function returns subtree structurally equal to 'newNode' subtree,
    // doing as little change as possible. It may return same 'oldNode', if remaking is not necessary.
    // It applies itself to subnodes recursively.
    // 
    // Note:
    //      - 'oldNode' is REFERENCE to shared ptr! Because it might remake the node and drop old value.
    //      - 'oldNode' may be nullptr.
    //      - It will assert that arguments do have same name, because the point is we are updating same variable node
    //      - This also assumes there cant be 2 children with same name (as of now, there actually can be multiple in DataNode)
    static DataNodeShared tryUpdateOldSubtreeFromNew( DataNodeShared&            oldNode,
                                                      const DataNodeShared&      newNode,
                                                      ParentingDataOpt           oldNodeParenting,
                                                      TreeUpdateOperations&      operations)
    {
        SV_ASSERT(newNode);
        if (oldNode)
        {
            SV_ASSERT(oldNode->getName() == newNode->getName());
        }

        
        auto shouldRemakeOldNode = [&](std::string &remakeReason) -> bool
        {
            // 1. old node doesnt even exist
            if (!oldNode)
            {
                remakeReason = "NodeDoesntExist";
                return true;
            }

            const bool oldIsLeaf = oldNode->isLeaf();
            const bool newIsLeaf = newNode->isLeaf();

            // 2. comp/leaf mismatch
            if (oldIsLeaf != newIsLeaf)
            {
                remakeReason = "NodeCategoryMismatch";
                return true;
            }

            // 3. leaf type mismatch
            if (oldIsLeaf && !anyHoldSameType(oldNode->tryGetLeafvalue(), newNode->tryGetLeafvalue()))
            {
                remakeReason = "LeafAnyTypeMismatch";
                return true;
            }

            // 4. different creation string, which we only check for leaves for now, cause we dont
            // want to remake comp nodes which only had their tab changed in widget options (cuz theres nothing else).
            // This decision is a bit controversial but should be fine.
            if (oldIsLeaf && !operations.nodesHaveSameCreationStringInOptions(oldNode, newNode))
            {
                remakeReason = "LeafCreationStringMismatch";
                return true;
            }

            //no mismatches
            return false;
        };

        std::string remakeReason;
        if (shouldRemakeOldNode(remakeReason))
        {
            auto reconstructedNode = DataNode::makeCopy(newNode);
            if (oldNode)
            {
                SV_LOG("upd", std::format("Replacing node due to [{}]: {}", remakeReason, newNode->getName()));
                operations.beforeReplacingNode(oldNode, newNode, reconstructedNode, oldNodeParenting);
            }
            else
            {
                SV_LOG("upd", std::format("Creating completely new node due to [{}]: {}", remakeReason, newNode->getName()));
                operations.beforeAddingCompletelyNewNode(reconstructedNode, oldNodeParenting, newNode);
            }
            return reconstructedNode;
        }

        // Now we are sure oldNode is not nullptr, and we can run some asserts:
        SV_ASSERT(oldNode);
        const bool oldIsLeaf = oldNode->isLeaf();
        const bool newIsLeaf = newNode->isLeaf();
        SV_ASSERT(oldIsLeaf == newIsLeaf);

        //Now we update comp node content, but for leaves we dont do anything else
        if (oldIsLeaf && newIsLeaf)
        {
            SV_LOG("upd", std::format("Leaf was not remade, used existing: {}", oldNode->getName()));
            return oldNode;
        }

        SV_LOG("upd", std::format("Comp was not remade, so gonna update children: {}", oldNode->getName()));

        std::vector<DataNodeShared> updatedChildrenListForOldNode;

        //So we can track which child in old node we need to delete afterwards
        std::vector<bool> oldChildrenStillExistsRegistry(oldNode->childrenCount(), false);

        for (int newChildIndex = 0; newChildIndex < newNode->childrenCount(); ++newChildIndex)
        {
            DataNodeShared newChild = newNode->tryGetChild(newChildIndex);

            //May not exist, and its fine
            int             oldChildWithSameNameIndex = -1;
            DataNodeShared  oldChildWithSameName      = oldNode->tryGetCompositeData()->getChild(newChild->getName(), &oldChildWithSameNameIndex);

            ParentingData   parentingOfUpdatedChild = { oldNode, newChildIndex };

            DataNodeShared  updatedChild              = tryUpdateOldSubtreeFromNew( oldChildWithSameName,
                                                                                    newChild,
                                                                                    parentingOfUpdatedChild,
                                                                                    operations );

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
                operations.beforeRemovingNode(oldChildToBeDeleted);
            }
        }

        // The line that actually removes those 'to be deleted' children is just this one.
        // (assuming nothing else holds their shared pointers, these children will go out of scope now)
        oldNode->tryGetCompositeData()->setChildren(updatedChildrenListForOldNode, oldNode);

        return oldNode;
    }

public:
    // The most simple version which will not even compare widget options
    // or do anything about widgets at all. Used for tests
    static DataNodeShared tryUpdateOldSubtreeFromNew(DataNodeShared&        oldNode,
                                                     const DataNodeShared&  newNode)
    {
        auto emptyOptionsOld = MapOfWidgetOptionsForNodes();
        auto emptyOptionsNew = MapOfWidgetOptionsForNodes();
        auto operations = std::make_unique<UpdateOperationsForPlainTreeWithoutWidgets>(std::ref(emptyOptionsOld), 
                                                                                       std::cref(emptyOptionsNew));

        return tryUpdateOldSubtreeFromNew(oldNode, newNode, {}, *operations);
    }

private:

};