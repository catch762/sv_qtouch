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
#include "Registrations/QTouchRegistrations.h"

#include "BasicTokenizer/AdhocTests.h"

#include "RunningTests.h"

void testRunQtouchApp()
{
    QDir projdir("doc/exampletestproject");
    //QString codeFilePath = projdir.absoluteFilePath("glsl_example_simple.h");
    QString codeFilePath = projdir.absoluteFilePath("glsl_example.h");

    auto app = new QTouchApp();

    app->openProjectDir(projdir);
    
    //app->loadTreeAndWidgetsFromCode({codeFilePath});

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
    QTouchAppTypesMetadata::registerEverything();

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



    auto res = app.exec();
    
    Logger::instance().logAppExitMessage(res);

    return res;
}