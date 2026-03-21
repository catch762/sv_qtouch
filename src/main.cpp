#include <QApplication>
#include <QPushButton>
#include <QTreeView>
#include <QHeaderView>
#include <any>

//#include "sv_common.h"
//#include "sv_qtcommon.h"
#include "sv_datalayer.h"
#include "TypesAndWidgets/LimitedValue/Internal/BaseXYPadWidget.h"

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

    auto w = WidgetMakerSystem::instance().makeWidgetForNode(root);
    w->show();
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
}

void testpad()
{
    auto M = new QWidget;
    auto lay = new QHBoxLayout(M);

    auto w = new BaseXYPadWidget();
    lay->addWidget(w);

    lay->addWidget(new QLineEdit("hellooooo"));

    auto b = new QPushButton("Kek");
    b->setMaximumWidth(150);

    lay->addWidget(b);

    M->show();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Logger::instance().logAppLaunchMessage();

    //AdhocTesting::runTest();

    test_widgets();

    //test_vec();

    //testpad();

    auto res = app.exec();
    
    //todo add log 
    qDebug() << "exit " << res;
    return res;
}