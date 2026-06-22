#include <QApplication>
#include <QPushButton>
#include <iostream>
#include <format> // Pure C++20 feature
#include "sv_common.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Using C++20 std::format to construct a string for the console
    std::string message = std::format("Hello World from C++20 and Qt {}!\n", getCurrentTimeHMS());
    std::cout << message;

    QPushButton button("Click to Close C++20 Window");
    button.resize(350, 120);



    // Connect the button click directly to exit the app safely
    QObject::connect(&button, &QPushButton::clicked, &app, &QApplication::quit);
    button.show();

    return app.exec();
}