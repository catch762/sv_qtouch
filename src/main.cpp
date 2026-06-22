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
#include "TreeAndWidgetsBuilder.h"
#include "QTouchApp.h"

#include "Registrations/Registrations.h"

#include "BasicTokenizer/AdhocTests.h"

#include "RunningTests.h"


/*
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

    auto treeJsonWithoutWidgets = SerializerForDataNodeTreeAndItsWidgets().toJson(root).value_or(QJsonValue());

    if (false)
    {
        SV_LOG("BEGIN treeJsonWithoutWidgets");
        SV_LOG(jsonValueToString(treeJsonWithoutWidgets).toStdString());
        SV_LOG("END treeJsonWithoutWidgets");
    }

    root.reset();

    SV_LOG("Deserializing from json beign: ");

    auto [newRoot, newRootWidget] = SerializerForDataNodeTreeAndItsWidgets().jsonToRootNodeAndItsWidget(treeJsonWithoutWidgets);

    if (qVariantHasWidget(newRootWidget))
    {
        auto widget = getWidgetFromQVariant(newRootWidget);
        widget->show();
    }
    else SV_ERROR("Well newRootWidget is empty");


    auto fullTreeJson = SerializerForDataNodeTreeAndItsWidgets().toJson(newRoot).value_or(QJsonValue());

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
        auto fullTreeJson = SerializerForDataNodeTreeAndItsWidgets().toJson(root).value_or(QJsonValue());

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
        std::tie(root, rootWidget) = SerializerForDataNodeTreeAndItsWidgets().jsonToRootNodeAndItsWidget(fullTreeJson);

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

TreeAndTopLevelWidgetsOpt test_SUP()
{
    if(auto data = SUP_DataParser().parseFiles({"C:/home/code/sv_qtouch/doc/glsl_example_simple.h"}))
    {
        SV_LOG(std::format("Parsed data successfully: {}", data->toString()));

        auto res = TreeAndWidgetsBuilder::buildTreeAndWidgets(*data);
        if (!res)
        {
            SV_ERROR("Well got nothing");
            return {};
        }

        QWidget *www = new QWidget;
        QVBoxLayout *lay = new QVBoxLayout(www);
        for (auto item : res->second)
        {
            if (auto w = getWidgetFromQVariant(item))
                lay->addWidget(w);
        }

        www->show();

        return res;
    }
    else
    {
        SV_ERROR("Couldnt parse data from glsl");
        return {};
    }
}
*/


void testRunQtouchApp()
{
    QDir projdir("doc/exampletestproject");
    QString codeFilePath = projdir.absoluteFilePath("glsl_example_simple.h");

    auto app = new QTouchApp();

    app->openProjectDir(projdir);
    
    app->loadTreeAndWidgetsFromCode({codeFilePath});

    app->show();
};

void testRadio()
{
    auto ww = new QWidget;
    auto lay = new QHBoxLayout(ww);

    for (int i = 0;i < 2; ++i)
    {
        auto w = new EnumWidget();
        w->setValue(
        Enum({
            {0, "hi"},
            {1, "aaa"},
            {1, "xaaa"},
            {1, "axxaa"},
            {1, "afefaa"},
            {2, "bbb"}}, 
            0)
        );
        lay->addWidget(w);

        //if (i==0) applyWidgetBorder(w);
    }

    ww->show();
}

int main(int argc, char *argv[])
{
    Logger::instance().logAppLaunchMessage();
    
    DatalayerDefaultTypesMetadata::registerEverything();

    QApplication app(argc, argv);

    //TESTS:
    {
        bool shouldExitAfterTests;
        int testsRes = runTests(argc, argv, shouldExitAfterTests);
        if(shouldExitAfterTests)
        {
            return testsRes;
        }
    }


    if(false)
    {
        QPalette p = app.palette();
        p.setColor(QPalette::Accent, QColor("#1499ff")); 
        app.setPalette(p);
    }

    if(false)
    {
        testRadio();
    }

    if(1)
    {
        
        //makePaletteDisplayWidget(app.palette())->show();
        testRunQtouchApp();
    }

    //testvisitor();

    //auto reswwww = test_SUP();
    //if(auto res = test_SUP())
    //{
    //    getWidgetFromQVariant(res->widget)->show();
    //}

    auto res = app.exec();
    
    Logger::instance().logAppExitMessage(res);

    return res;
}