#include <QApplication>
#include <QPushButton>
#include <QTreeView>
#include <QHeaderView>
#include <any>

//#include "sv_common.h"
//#include "sv_qtcommon.h"
#include "sv_datalayer.h"


void test3()
{
    auto root       = std::make_shared<DataNode>(QString("root"), DataNode::NodeType::Composite);
    auto child      = std::make_shared<DataNode>(QString("child"), DataNode::NodeType::Composite);
    auto grandchild = std::make_shared<DataNode>(QString("grandchild"), DataNode::NodeType::Leaf);
    *grandchild->tryGetLeafvalue() = QString("okookko");
    root->addChild(child);
    child->addChild(grandchild);


    DataLayerSystem::instance().registerWidgetMaker<QString>(
        [](DataNodeShared node) -> QWidget*
        {
            if (!DataLayerSystem::instance().checkIsProperNodeForCreatingWidgetOfType<QString>(node))
            {
                return nullptr;
            }

            auto *widget = new QLineEdit(node->tryGetLeafvalue()->toString());

            auto nodeWeak = DataNodeWeak(node);

            //todo check if 3rd param...
            QObject::connect(widget, &QLineEdit::textChanged, widget, [nodeWeak](const QString &s)
            {
                if (auto nodeShared = nodeWeak.lock())
                {
                    if (auto leaf = nodeShared->tryGetLeafvalue())
                    {
                        *leaf = s;
                    }
                    else
                    {
                        SV_ASSERT(false);
                    }
                }
            });

            return widget;
        }
    );

    auto w = DataLayerSystem::instance().getWidgetForNode(grandchild);
    w->show();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    SV_LOG("\n\n--- app launch --- " + getCurrentTimeHMS() + "\n")

    test3();

    auto res = app.exec();
    qDebug() << "app.exec() returned " << res;
}