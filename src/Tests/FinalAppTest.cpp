#include "sv_qtcommon.h"
#include "../QTouchApp.h"
#include "doctest/doctest.h"
#include "WidgetLogic/WidgetsForNodeManager.h"
#include "DataTypesAndTheirWidgets/DataTypesAndTheirWidgets.h"

TEST_CASE("Running entire app")// * doctest::skip())
{
    //SV_WARN("gonnarun");

    QDir projdir("doc/exampletestproject");
    QString codeFilePath = projdir.absoluteFilePath("glsl_example_simple.h");

    //so it deletes upon exiting, if REQUIRE failed and we dont get to manual cleanup
    auto app = std::make_unique<QTouchApp>();

    REQUIRE(app->openProjectDir(projdir));

    REQUIRE(app->loadTreeAndWidgetsFromCode({codeFilePath}));

    auto getFinalWidget = [&]()->LimitedValueVecWidget*
    {
        auto root = app->getRootNode();
        REQUIRE(root);

        auto kidForVec4 = root->tryGetChild(1);
        REQUIRE(kidForVec4);

        auto nodeWidgetForVec4 = WidgetsForNodeManager::getSaveablePrimaryWidgetForNode(kidForVec4);
        REQUIRE(nodeWidgetForVec4);

        QWidget* widgetForVec4 = nodeWidgetForVec4->getContentWidget(0);
        REQUIRE(widgetForVec4);

        return dynamic_cast<LimitedValueVecWidget*>(widgetForVec4);
    };

    //Change mode of widget (its not data, but its widget options)
    {
        auto limVecWidget = getFinalWidget();
        REQUIRE(limVecWidget);

        REQUIRE(limVecWidget->getMode() == LimitedValueVecWidget::Mode::ShowJustLimitedValueWidgets);

        limVecWidget->setMode(LimitedValueVecWidget::Mode::ShowXYPad);
    }

    //Save everything to preset (it also should save this widget options)
    const QString tempPresetName = "__TEMP_TEST_PRESET.json";
    {
        REQUIRE(app->savePreset(tempPresetName));
    }

    //Load back from saved preset and check that widget options were loaded as well
    {
        SV_LOG("Loading temp preset back:");

        REQUIRE(app->loadTreeAndWidgetsUsingPresetFileName(tempPresetName));

        auto limVecWidget = getFinalWidget();
        REQUIRE(limVecWidget);

        REQUIRE(limVecWidget->getMode() == LimitedValueVecWidget::Mode::ShowXYPad);
    }

    //cleanup preset file that we saved:
    if (auto presetsDir = app->getPresetsSubdir())
    {
        bool deleted = QFile::remove(presetsDir->absoluteFilePath(tempPresetName));
    }

    SV_LOG("Final test succeeded");
}