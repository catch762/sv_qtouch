#include "QTouchApp.h"
#include "WidgetLogic/WidgetsForNodeManager.h"
#include "DataNode\DataNodeSerializers.h"
#include "Interpolation/InterpolationSystem.h"

#include "UIComponents/TopLevelWidgetsContainer.h"
#include "UiComponents/PresetFileView.h"
#include "UiComponents/PresetTab.h"
#include <QMenuBar>
#include <QFileDialog>

QTouchApp::QTouchApp(QWidget *parent) : QMainWindow(parent)
{
    centralWidget = new QWidget(this);
    centralLayout = new QHBoxLayout(centralWidget);
    setCentralWidget(centralWidget);

    {
        widgetsView = new TopLevelWidgetsContainer(this);
        centralLayout->addWidget(widgetsView);
    }

    {
        presetTab = new PresetTab(centralWidget);
        //presetTab->getPresetView()->setRootPath(getPresetsSubdir().absolutePath());
        connect(presetTab, &PresetTab::presetMixingActivated, this, &QTouchApp::onPresetMixingActivated);
        connect(presetTab, &PresetTab::presetMixingDeactivated, this, [this]()
        {
            setTreeType(TreeType::Standalone); //yes thats all we do    
        });
        connect(presetTab, &PresetTab::presetSavingRequested, this, &QTouchApp::onPresetSavingRequested);
        connect(presetTab->getPresetView(), &PresetFileView::presetLoadingRequested,
                this, [this](const QString& presetFilename)
                {
                    if (!requireProjectIsOpenedFor("Preset loading")) return;

                    loadTreeAndWidgetsFromPresetFile(getPresetsSubdir()->filePath(presetFilename));
                });

        centralLayout->addWidget(presetTab);
    }

    initMenuBar();

    closeProject();
}

bool QTouchApp::loadTreeAndWidgetsFromCode(const QStringVec &codeFilePaths)
{
    if (!requireProjectIsOpenedFor("loadTreeAndWidgetsFromCode")) return false;

    deleteExistingTreeAndAllWidgets();
    setTreeType(TreeType::Standalone);

    auto parsedVarData = SUP_DataParser().parseFiles(codeFilePaths);
    if (!parsedVarData)
    {
        SV_MSGBOX_ERROR("QTouchApp: failed to parse GLSL code.");
        return false;
    }
    else
    {
        SV_LOG(parsedVarData->toString());
    }

    auto rootNodeAndTopLevelWidgets = TreeAndWidgetsBuilder::buildTreeAndWidgets(*parsedVarData);
    if (!rootNodeAndTopLevelWidgets)
    {
        SV_MSGBOX_ERROR("QTouchApp: failed to create root node and top level widgets for GLSL code variables.");
        return false;
    }
    else{
        rootNode = std::move(rootNodeAndTopLevelWidgets->first);
        widgetsView->setTopLevelWidgets(std::move(rootNodeAndTopLevelWidgets->second));
        widgetsView->setEnabled(true);
    }

    return true;
}

bool QTouchApp::loadTreeAndWidgetsFromPresetFile(const QString &filePath)
{
    if (!requireProjectIsOpenedFor("loadTreeAndWidgetsFromPresetFile")) return false;

    deleteExistingTreeAndAllWidgets();
    setTreeType(TreeType::Standalone);

    SV_LOG(std::format("Loading preset file: {}", filePath));
    
    if (auto treeAndWidgets = createTreeAndWidgetsFromFile(filePath))
    {
        rootNode = std::move(std::get<0>(*treeAndWidgets));
        widgetsView->setTopLevelWidgets(std::move(std::get<1>(*treeAndWidgets)));
        widgetsView->setEnabled(true);
    }
    else
    {
        SV_MSGBOX_ERROR(std::format("Error loading from preset file: {}", filePath));
        return false;
    }

    return true;
}

void QTouchApp::onPresetSavingRequested(const QString &presetFilename)
{
    if (!requireProjectIsOpenedFor("Preset saving")) return;

    if(!rootNode)
    {
        SV_ERROR("Save preset failed: rootNode is empty");
        return;
    }

    //if it existed, we already got confirmation to delete old file

    auto file = QFileInfo( getPresetsSubdir()->absoluteFilePath(presetFilename) );
    
    auto json = SerializerForDataNodeTreeAndItsWidgets::toJson(rootNode);
    if (!json)
    {
        SV_ERROR("Save preset failed: serialization to json has failed")
        return;
    }

    bool saved = saveJsonValueToFile(*json, file.absoluteFilePath());
    if (!saved)
    {
        SV_ERROR("Save preset failed: writing to file has failed");
        return;
    }

    SV_LOG(std::format("Successfully saved preset to {}", file.absoluteFilePath()));
}

QDirOpt QTouchApp::getProjectDir()
{
    return projectDir;
}

QDirOpt QTouchApp::getPresetsSubdir()
{
    if (projectDir) return QDir(projectDir->filePath("presets"));
    else return {};
}

void QTouchApp::setTreeType(TreeType type)
{
    if (type != rootType)
    {
        SV_LOG(std::format("Setting tree type: {}", rootType == TreeType::IsMixResult ? "IsMixResult" : "Standalone"));

        rootType = type;

        bool mixingModeEnabled = rootType == TreeType::IsMixResult;
        presetTab->setMixPresetMode(mixingModeEnabled, false); //setting just visual button state
    }
}

bool QTouchApp::openProjectDir(const QDir &newProjectDir)
{
    if (!newProjectDir.exists())
    {
        SV_MSGBOX_ERROR(std::format("Cant open project folder which doesnt exist.\n"
                                    "Project folder: [{}]", newProjectDir.absolutePath()));
        closeProject();
        return false;
    }

    if (projectDir && *projectDir == newProjectDir)
    {
        SV_MSGBOX_LOG(std::format("You already had this project folder open, this will not do anything.\n"
                                  "Project folder: [{}]", projectDir->absolutePath()));
        return false;
    }

    closeProject();

    projectDir = newProjectDir;

    if (!getPresetsSubdir()->exists())
    {
        getPresetsSubdir()->mkdir(".");
        SV_LOG("Created folder " + getPresetsSubdir()->absolutePath().toStdString());
    }

    presetTab->getPresetView()->setRootPath(getPresetsSubdir()->absolutePath());
    //presetTab->setEnabled(true);

    centralWidget->setEnabled(true);
    closeProjectAction->setEnabled(true);
    loadCodeAction->setEnabled(true);

    SV_LOG(std::format("Successfully opened project folder [{}]", projectDir->absolutePath()));
    return true;
}

void QTouchApp::deleteExistingTreeAndAllWidgets()
{
    rootNode.reset();
    widgetsView->deleteAllTopLevelWidgets();
    widgetsView->setEnabled(false);

    WidgetsForNodeManager::clear();
}

std::optional<std::tuple<DataNodeShared, NodeWidgetVec>> QTouchApp::createTreeAndWidgetsFromFile(const QString &filePath)
{
    auto json = loadJsonFromFile(filePath);
    if (!json)
    {
        return {};
    }

    auto res = SerializerForDataNodeTreeAndItsWidgets().jsonToRootNodeAndTopLevelChildrenWidgets(*json);

    if (!std::get<0>(res)) return {};

    return res;
}

void QTouchApp::initMenuBar()
{
    auto* projectMenu = menuBar()->addMenu("Project");
    projectMenu->addAction("Create or open existing project", this, &QTouchApp::createOrOpenProjectAction);

    {
        closeProjectAction = new QAction("Close project", this);
        connect(closeProjectAction, &QAction::triggered, this, &QTouchApp::closeProject);

        projectMenu->addAction(closeProjectAction);
    }

    {
        loadCodeAction = new QAction("Load GLSL code", this);
        connect(loadCodeAction, &QAction::triggered, this, [this]()
        {
            if (!requireProjectIsOpenedFor("Load GLSL code")) return;

            const QString codeFilePath = QFileDialog::getOpenFileName(
                this,
                tr("Open code file"),
                getProjectDir().value().absolutePath(),
                tr("All Files (*)")
            );

            if (codeFilePath.isEmpty())
            {
                return;
            }

            loadTreeAndWidgetsFromCode({codeFilePath});
        });

        projectMenu->addAction(loadCodeAction);
    }
}

void QTouchApp::createOrOpenProjectAction()
{
    QString folder = QFileDialog::getExistingDirectory(
        this,
        tr("Select project folder"),
        QDir::homePath(),
        /*QFileDialog::ShowDirsOnly |*/ QFileDialog::DontResolveSymlinks
    );

    if (folder.isEmpty())
    {
        SV_LOG("Cancelling createOrOpenProject, user did not specify a folder");
        return;
    }

    openProjectDir(QDir(folder));
}

void QTouchApp::closeProject()
{
    projectDir = QDir();

    deleteExistingTreeAndAllWidgets(); //disables widgetsView
    setTreeType(TreeType::Standalone);
    centralWidget->setDisabled(true);

    closeProjectAction->setDisabled(true);
    loadCodeAction->setDisabled(true);
}

bool QTouchApp::projectIsOpened()
{
    return projectDir.has_value();
}

bool QTouchApp::requireProjectIsOpenedFor(const char *forOperation, bool withMsgBox)
{
    bool isOpened = projectIsOpened();

    if (!isOpened)
    {
        auto err = std::format("Operation [{}] failed, because it requires "
                               "project to be opened, and it is not.", forOperation);
        if (withMsgBox)
        {
            SV_MSGBOX_ERROR(err);
        }
        else
        {
            SV_ERROR(err);
        }
    }

    return isOpened;
}

void QTouchApp::onPresetMixingActivated(const QString& presetFilenameA,
                                        const QString& presetFilenameB,
                                        double morphAtoB01)
{
    // This is called when:
    //  - preset mixing is turned on
    //  - user moves mix slider to a different position.
    //
    // A lot of the operations here are expensive - trees may be large, constructing
    // or even comparing trees is a heavy task. We have to minimize work, whenever possible.
    //
    // For example, if user switched preset mixing on, off, and on again - nothing REALLY changes,
    // so we have to reuse existing trees, not reconstruct them.

    if (!requireProjectIsOpenedFor("Preset mixing")) return;

    //Ensure presets A and B are loaded:
    auto presetDir = getPresetsSubdir();

    auto resultA = presetForMixing_A.loadFileIfItsNotLoadedYet(*presetDir, presetFilenameA);
    auto resultB = presetForMixing_B.loadFileIfItsNotLoadedYet(*presetDir, presetFilenameB);

    if ( resultA == LoadedPreset::Result::Error ||
         resultB == LoadedPreset::Result::Error )
    {
        SV_MSGBOX_ERROR("Preset mixing failed, couldnt load A/B presets");
        setTreeType(TreeType::Standalone);
        return;
    }

    const bool alreadyHadAAndBLoaded =  resultA == LoadedPreset::Result::AlreadyHadThisFile &&
                                        resultB == LoadedPreset::Result::AlreadyHadThisFile;

    if (!alreadyHadAAndBLoaded)
    {                                    
        if (!treesAreStructurallyEqual_withMismatchLog(*presetForMixing_A.rootNode, *presetForMixing_B.rootNode))
        {
            SV_MSGBOX_ERROR("Preset mixing failed, A/B trees not structurally equal");
            setTreeType(TreeType::Standalone);
            return;
        }
    }

    bool existingRootNodeIsAlreadyCorrectMixingTarget = alreadyHadAAndBLoaded && rootType == TreeType::IsMixResult;

    //Check if actually existing rootNode happens to have correct structure to become A/B mix result
    //In this case we reuse it
    if (!existingRootNodeIsAlreadyCorrectMixingTarget && rootNode)
    {
        if (treesAreStructurallyEqual(*presetForMixing_A.rootNode, *rootNode))
        {
            SV_LOG("Preset mixing: Reused existing rootNode tree, it matches preset A tree");
            setTreeType(TreeType::IsMixResult);
            existingRootNodeIsAlreadyCorrectMixingTarget = true;
        }
    }

    //Ok, we cant reuse it.
    if (!existingRootNodeIsAlreadyCorrectMixingTarget)
    {
        deleteExistingTreeAndAllWidgets();

        auto treeAndWidgets = createTreeAndWidgetsFromFile(presetDir->filePath(presetForMixing_B.fileName));
        if (!treeAndWidgets)
        {
            SV_MSGBOX_ERROR("Couldnt make mixing target (tree made from preset B)");
            setTreeType(TreeType::Standalone);
            return;
        }

        setTreeType(TreeType::IsMixResult);
        rootNode        = std::move(std::get<0>(*treeAndWidgets));
        widgetsView->setTopLevelWidgets(std::move(std::get<1>(*treeAndWidgets)));
        widgetsView->setEnabled(true);
    }

    // ok, now 'rootNode' has valid tree with structure as in both presets,
    // 'topLevelWidgets' is also valid and populated,
    // now we just mix presets in that 'rootNode' tree.
    bool interpSuccess = InterpolationSystem::interpolateTwoTreesToThird(*presetForMixing_A.rootNode,
                                                                         *presetForMixing_B.rootNode,
                                                                         *rootNode,
                                                                         morphAtoB01);
    if(!interpSuccess)
    {
        SV_MSGBOX_ERROR(std::format("InterpolationSystem failed to interpolate two trees to third.\n"
                             "A  : {}\n"
                             "B  : {}\n"
                             "Res: {}\n",
                             presetForMixing_A.rootNode, presetForMixing_B.rootNode, rootNode));
        setTreeType(TreeType::Standalone);
        return;
    }

    SV_LOG("Preset mixing: interpolated trees successfully. Updating widgets:"); 

    //We just updated DataNode tree, now trigger widget updates from tree model:
    DataNode::iterateRecoursively(rootNode, [](const DataNodeShared& node)
    {
        WidgetsForNodeManager::updateAllWidgetsFromNodeState(node);
    });
}

QTouchApp::LoadedPreset::Result QTouchApp::LoadedPreset::loadFileIfItsNotLoadedYet(const QDir &presetsDir, const QString &presetFileName)
{
    if (rootNode && fileName == presetFileName)
    {
        return LoadedPreset::Result::AlreadyHadThisFile;
    }

    QString filePath = presetsDir.filePath(presetFileName);
    if (!QFile::exists(filePath))
    {
        SV_ERROR(std::format("Load preset from [{}] failed: file doesnt exist", filePath));

        clear();
        return LoadedPreset::Result::Error;
    }

    auto json = loadJsonFromFile(filePath);
    if (!json)
    {
        SV_ERROR(std::format("Load preset from [{}] failed: couldnt parse json from file", filePath));

        clear();
        return LoadedPreset::Result::Error;
    }

    rootNode = DataNode::fromJSON(json.value());
    if (!rootNode)
    {
        SV_ERROR(std::format("Load preset from [{}] failed: couldnt parse data tree", filePath));

        clear();
        return LoadedPreset::Result::Error;
    }

    //Successfully loaded then.
    fileName = presetFileName;
    return LoadedPreset::Result::JustLoadedThisFile;
}

void QTouchApp::LoadedPreset::clear()
{
    rootNode.reset();
    fileName.clear();
}
