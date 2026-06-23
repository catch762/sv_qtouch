#include "sv_qtcommon.h"
#include "../QTouchApp.h"
#include "doctest/doctest.h"

TEST_CASE("Running entire app" * doctest::skip())
{
    //SV_WARN("gonnarun");

    QDir projdir("doc/exampletestproject");
    QString codeFilePath = projdir.absoluteFilePath("glsl_example_simple.h");

    auto app = new QTouchApp();

    REQUIRE(app->openProjectDir(projdir));

    REQUIRE(app->loadTreeAndWidgetsFromCode({codeFilePath}));

    delete app;
}