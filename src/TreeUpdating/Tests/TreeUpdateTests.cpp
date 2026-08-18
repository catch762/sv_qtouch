#include "TreeUpdating/TreeUpdater.h"
#include "doctest/doctest.h"

using namespace datanode_helpers;

DataNodeShared baseTree()
{
	return	dncomp("", {
				dnleaf("a", int(100)),
				dncomp("b", {
					dnleaf("c", int(300)),
					dnleaf("d", int(400))
				}),
				dnleaf("e", int(200))
			});
}

TEST_CASE("Updating tree: simple insert new nodes")
{
	auto oldTree = baseTree();
	auto newTree = baseTree();
	{
		newTree->tryGetChild("b")->addChild(
			dnleaf("XXX_1", int(999)),
			1 //insert at index 1, in the middle
		);

		newTree->addChild(
			dnleaf("XXX_2", int(999))
		);
	}

	TreeUpdater::tryUpdateOldSubtreeFromNew(oldTree, newTree);

	CHECK(treesAreStructurallyEqual(*oldTree.get(), *newTree.get()));
}

TEST_CASE("Updating tree: renames, insert, delete")
{
	auto oldTree = baseTree();
	auto newTree = baseTree();
	{
		auto b = newTree->tryGetChild("b");
		auto c = b->tryGetChild("c");

		//1) rename b
		b->setName("b_renamed");

		//2) rename c
		c->setName("c_renamed");

		//3) add new child to b
		b->addChild(
			dnleaf("XXX_1", int(999)),
			1
		);

		//4) delete last child (e)
		auto children = newTree->tryGetCompositeData()->getChildren();
		children.pop_back();
		newTree->tryGetCompositeData()->setChildren(children, newTree);
	}

	TreeUpdater::tryUpdateOldSubtreeFromNew(oldTree, newTree);

	CHECK(treesAreStructurallyEqual(*oldTree.get(), *newTree.get()));
}