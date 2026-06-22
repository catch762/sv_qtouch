#include <QApplication>
#include <QPushButton>
#include <iostream>
#include <format> // Pure C++20 feature
#include "sv_common.h"
#include "sv_qtcommon.h"
#include "sv_datalayer.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Using C++20 std::format to construct a string for the console
    std::string message = std::format("Hello World from C++20 and Qt {}!\n", getCurrentTimeHMS());
    std::cout << message;

    QPushButton button("Click to Close C++20 Window");
    button.resize(350, 120);

    auto l = DataNode::makeLeaf("adadw", 5);

    auto w = new HorizontalScrollAreaWidget(30);
    w->resize(350, 100);
    w->show();

    // Connect the button click directly to exit the app safely
    QObject::connect(&button, &QPushButton::clicked, &app, &QApplication::quit);
    button.show();

    return app.exec();
}