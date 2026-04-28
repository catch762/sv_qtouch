#include <QApplication>
#include <QPushButton>
#include <QTreeView>
#include <QHeaderView>
#include <any>

//#include "sv_common.h"
//#include "sv_qtcommon.h"
#include "sv_datalayer.h"
#include "DataTypesAndTheirWidgets/DataTypesAndTheirWidgets.h"
#include "DataTypesAndTheirWidgets/Bool/BoolVecWidget.h"
#include "WidgetLogic/WidgetsForNodeManager.h"

#include "SUP_Data/SUP_DataParser.h"

DataNodeShared makeSimpleTree()
{
    auto root   = DataNode::makeComposite("root");

    auto child_a = root->addLeaf("child_a", LimitedIntVec{
        LimitedInt{50, 0, 100}, LimitedInt{}
    });

    auto child_b = root->addLeaf("child_b", BoolVec{true,false,false,true,true});

    return root;
}

void test_nodes_and_widgets_ThroughJson()
{
    auto root = makeSimpleTree();

    auto treeJsonWithoutWidgets = SerializerForDataNodeTreeAndItsWidgets().toJson(root);

    if (false)
    {
        SV_LOG("BEGIN treeJsonWithoutWidgets");
        SV_LOG(jsonValueToString(treeJsonWithoutWidgets).toStdString());
        SV_LOG("END treeJsonWithoutWidgets");
    }

    root.reset();

    SV_LOG("Deserializing from json beign: ");

    auto [newRoot, newRootWidget] = SerializerForDataNodeTreeAndItsWidgets().fromJson(treeJsonWithoutWidgets);

    if (qVariantHasWidget(newRootWidget))
    {
        auto widget = getWidgetFromQVariant(newRootWidget);
        widget->show();
    }
    else SV_ERROR("Well newRootWidget is empty");


    auto fullTreeJson = SerializerForDataNodeTreeAndItsWidgets().toJson(newRoot);

    if (true)
    {
        SV_LOG("BEGIN fullTreeJson");
        SV_LOG(jsonValueToString(fullTreeJson).toStdString());
        SV_LOG("END fullTreeJson");
    }

    SV_LOG("test_nodes_and_widgets_ThroughJson end;");
}

void test_nodes_and_widgets()
{
    auto root = makeSimpleTree();
    
    auto widgetRoot = WidgetMakerSystem::instance().createAndRegisterWidgetForNode(root);
    if (qVariantHasWidget(widgetRoot))
    {
        auto widget = getWidgetFromQVariant(widgetRoot);
        widget->show();
    }
    else SV_ERROR("Well widgetRoot is empty");

    

    auto next = [=]() mutable
    {
        auto fullTreeJson = SerializerForDataNodeTreeAndItsWidgets().toJson(root);

        if (true)
        {
            SV_LOG("BEGIN fullTreeJson");
            SV_LOG(jsonValueToString(fullTreeJson).toStdString());
            SV_LOG("END fullTreeJson");
        }

        root.reset();
        delete getWidgetFromQVariant(widgetRoot);
        WidgetsForNodeManager::clear();

        QVariantHoldingWidget rootWidget;
        std::tie(root, rootWidget) = SerializerForDataNodeTreeAndItsWidgets().fromJson(fullTreeJson);

        if (auto w = getWidgetFromQVariant(rootWidget))
        {
            w->show();
        }
        else SV_ERROR("Eh, rootWidget null ?");
    };

    auto b = new QPushButton("SAVE, DELETE ALL AND RELOAD");
    b->setFixedSize(300, 200);
    b->show();
    QObject::connect(b, &QPushButton::clicked, next);


    SV_LOG("test_nodes_and_widgets end;");
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Logger::instance().logAppLaunchMessage();

    if(false)
    {
        QPalette p = app.palette();
        p.setColor(QPalette::Accent, QColor("#1499ff")); 
        app.setPalette(p);
    }

    //AdhocTesting::runTest();

    //test_widgets();

    //makePaletteDisplayWidget(app.palette())->show();
    //createThemeIconsWidget()->show();

    //test_nodes_and_widgets();
    //booltest();
    
    SUP_DataParser::Test();
    
    
    //testjustvec();

    //test_vec();

    //testpad();

    auto res = app.exec();
    
    Logger::instance().logAppExitMessage(res);

    return res;
}