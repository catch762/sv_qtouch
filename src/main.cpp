#include <QApplication>
#include <QPushButton>
#include <QTreeView>
#include <QHeaderView>
#include <any>

//#include "sv_common.h"
//#include "sv_qtcommon.h"
#include "sv_datalayer.h"



DataNodeShared getroot()
{
    auto root       = std::make_shared<DataNode>(QString("root"), DataNode::NodeType::Composite);
    auto child      = std::make_shared<DataNode>(QString("child"), DataNode::NodeType::Composite);
    auto grandchild = std::make_shared<DataNode>(QString("grandchild"), DataNode::NodeType::Leaf);
    *grandchild->tryGetLeafvalue() = QString("okookko");
    
    root->addChild(child);
    child->addChild(grandchild);

    return root;
}

void test2()
{
    auto root = getroot();

    // Create model and view
    DataTreeModel *model = new DataTreeModel(root);
    QTreeView *treeView = new QTreeView();
    //treeView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);  // ← CRITICAL
    treeView->setModel(model);
    treeView->expandAll();  // Optional: expand all nodes

    treeView->show();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    SV_LOG("just a msg")

    SV_LOG("cattt|", "just another msg")

    SV_WARN("warning")

    SV_ERROR("module", "error")

    SV_ASSERT(5==6)

    test2();

    auto res = app.exec();
    qDebug() << "app.exec() returned " << res;
}