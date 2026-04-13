#include <QApplication>
#include <QPushButton>
#include <QTreeView>
#include <QHeaderView>
#include <any>

//#include "sv_common.h"
//#include "sv_qtcommon.h"
#include "sv_datalayer.h"
#include "DataTypesAndTheirWidgets/LimitedValue/Internal/BaseXYPadWidget.h"
#include "DataTypesAndTheirWidgets/LimitedValue/Internal/XYPadWithPresetsWidget.h"

#include "WidgetLogic/WidgetsForNodeManager.h"


void test_widgets()
{
    auto root   = DataNode::makeComposite("root");

    auto childKek = root->addLeaf("childKek", LimitedDoubleVec{
        LimitedDouble{6, 5, 7}, LimitedDouble{50, 0, 100}, LimitedDouble{}
    });

    //auto child  = root->addComposite("child");
    //auto grand0 = child->addLeaf("grand0_name", QString("qstring text"));
    //auto grand1 = child->addLeaf("grand1_name", LimitedDouble{6, 5, 7});
    //auto grand2 = child->addLeaf("grand2_name", LimitedDoubleVec{
    //    LimitedDouble{6, 5, 7}, LimitedDouble{50, 0, 100}, LimitedDouble{}
    //});

    //auto w = WidgetMakerSystem::instance().makeWidgetForLeafNode(root);
    //w->show();

    auto w = WidgetMakerSystem::instance().createAndRegisterWidgetForNode(childKek);
    if (auto p = getWidgetFromQVariant(w))
    {
        p->show();
    }
}

void test_vec()
{
    LimitedDoubleVec vec{ LimitedDouble{}, LimitedDouble{2,2,2}};
    auto widget = new LimitedDoubleVecWidget(vec);
    QObject::connect(widget, &LimitedDoubleVecWidget::valueChanged, [](const auto &vec)
    {
        SV_LOG("Valchanged, size " + std::to_string(vec.size()));
    });

    //vec.push_back(LimitedDouble());
    vec.pop_back();
    widget->setValue(vec);

    widget->show();


    {
        QVariant v = QVariant::fromValue(widget);
        QVariant v2 = QVariant::fromValue((QWidget*)widget);
        SV_LOG(std::format("Ok its {} {} and {}", qVariantInfo(v) , qVariantInfo(v2), v.canConvert<XYPadWithPresetsWidget*>() ));
    }
}

void testpad()
{
    auto w = new XYPadWithPresetsWidget();
    w->show();
    return;

    auto M = new QWidget;
    auto lay = new QHBoxLayout(M);

    
    lay->addWidget(w);

    lay->addWidget(new QLineEdit("hellooooo"));

    auto b = new QPushButton("Kek");
    b->setMaximumWidth(150);

    lay->addWidget(b);

    M->show();
}

DataNodeShared makeSimpleTree()
{
    auto root   = DataNode::makeComposite("root");

    auto child_a = root->addLeaf("child_a", LimitedDoubleVec{
        LimitedDouble{6, 5, 7}, LimitedDouble{50, 0, 100}, LimitedDouble{}
    });

    auto child_b = root->addLeaf("child_b", LimitedDoubleVec{
        LimitedDouble{}, LimitedDouble{}, LimitedDouble{}
    });

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

    QPalette p = app.palette();
    //QPalette p = QPalette();
    p.setColor(QPalette::Accent, QColor("#1499ff")); 
    app.setPalette(p);

    //AdhocTesting::runTest();

    //test_widgets();

    //makePaletteDisplayWidget(app.palette())->show();
    //createThemeIconsWidget()->show();

    test_nodes_and_widgets();

    //test_vec();

    //testpad();

    auto res = app.exec();
    
    Logger::instance().logAppExitMessage(res);

    return res;
}