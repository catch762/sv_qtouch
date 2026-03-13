#include <QApplication>
#include <QPushButton>
#include <QTreeView>
#include <QHeaderView>
#include <any>

//#include "sv_common.h"
//#include "sv_qtcommon.h"
#include "sv_datalayer.h"
#include "DataLayerUtils.h"

void test_widgets()
{
    auto root   = DataNode::makeComposite("root");
    auto child  = root->addComposite("child");
    auto grand0 = child->addLeaf("grand0_name", QString("qstring text"));
    auto grand1 = child->addLeaf("grand1_name", LimitedDouble{});

    auto w = WidgetMakerSystem::instance().makeWidgetForNode(grand1);
    w->show();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Logger::instance().logAppLaunchMessage();

    //AdhocTesting::runTest();

    test_widgets();

    auto res = app.exec();
    
    //todo add log 
    qDebug() << "exit " << res;
    return res;
}