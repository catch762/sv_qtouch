#include <QApplication>
#include <QPushButton>
#include <QTreeView>
#include <QHeaderView>
#include <any>

//#include "sv_common.h"
//#include "sv_qtcommon.h"
#include "sv_datalayer.h"
#include "DataLayerUtils.h"

void test3()
{
    auto root       = std::make_shared<DataNode>(QString("root"), DataNode::NodeType::Composite);
    auto child      = std::make_shared<DataNode>(QString("child"), DataNode::NodeType::Composite);
    auto grandchild = std::make_shared<DataNode>(QString("grandchild"), DataNode::NodeType::Leaf);
    *grandchild->tryGetLeafvalue() = QString("okookko");
    root->addChild(child);
    child->addChild(grandchild);

    auto w = WidgetMakerSystem::instance().makeWidgetForNode(grandchild);
    w->show();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Logger::instance().logAppLaunchMessage();

    AdhocTesting::runTest();

    //test3();

    auto res = app.exec();
    qDebug() << "app.exec() returned " << res;
}