#include "QTouchApp.h"
#include "WidgetLogic/WidgetsForNodeManager.h"
#include "DataNode\DataNodeSerializers.h"
#include "Interpolation/InterpolationSystem.h"

#include "UIComponents/TopLevelWidgetsContainer.h"
#include "UiComponents/Presets/PresetFileView.h"
#include "UiComponents/Presets/PresetTab.h"
#include <QMenuBar>
#include <QFileDialog>

#include "DataToTDFormat\TDFormatTreeConverter.h"

namespace
{
    constexpr auto ProjectJsonFileName = "project.json";
    constexpr auto PresetExportListKey = "presetExportList";
};

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
        connect(presetTab, &PresetTab::presetSavingRequested, this, &QTouchApp::savePreset);
        connect(presetTab, &PresetTab::exportPresetsRequested, this, &QTouchApp::exportPresets);
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

    tdClient = new TDTcpClient(this);

    {
        QWidget* testui = new QWidget(nullptr);
        QVBoxLayout* lay = new QVBoxLayout(testui);

        QPushButton* doConnect = new QPushButton("connect to default");
        connect(doConnect, &QPushButton::clicked, [&]()
        {
            tdClient->connectToTd();
        });

        QPushButton* send = new QPushButton("send");
        connect(send, &QPushButton::clicked, [&]()
        {
            if (!rootNode)
            {
                SV_ERROR("root is null bro");
                return;
            }

            TreeAsVec4Array data;
            if (auto err = convertTreeToVec4Array(rootNode, data))
            {
                SV_ERROR(*err);
            }

            tdClient->sendTreeData(data, QTouchUITreePresetName);
        });

        lay->addWidget(doConnect);
        lay->addWidget(send);

        testui->show();
    }
}

bool QTouchApp::loadTreeAndWidgetsFromCode(const QStringVec &codeFilePaths)
{
    if (!requireProjectIsOpenedFor("loadTreeAndWidgetsFromCode")) return false;

    deleteExistingTreeAndAllWidgets();

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

bool QTouchApp::loadTreeAndWidgetsUsingPresetFileName(const PresetNameString& presetName)
{
    if (!requireProjectIsOpenedFor("loadTreeAndWidgetsUsingPresetFileName")) return false;

    auto filePath = absPathForPresetJsonFile(presetName);
    return loadTreeAndWidgetsFromPresetFile(filePath);
}

bool QTouchApp::savePreset(const PresetNameString& presetName) const
{
    if (!requireProjectIsOpenedFor("Preset saving")) return false;

    if(!rootNode)
    {
        SV_ERROR("Save preset failed: rootNode is empty");
        return false;
    }

    //if it existed, we already got confirmation to delete old files

    // 1) .json
    {
        auto json = SerializerForDataNodeTreeAndItsWidgets::toJson(rootNode);
        if (!json)
        {
            SV_ERROR("Save preset failed: serialization to json has failed")
                return false;
        }

        bool saved = saveJsonValueToFile(*json, absPathForPresetJsonFile(presetName));
        if (!saved)
        {
            SV_ERROR("Save preset failed: writing to json file has failed");
            return false;
        }
    }

    // 2) vec4 data file
    {
        TreeAsVec4Array treeAsVec4;
        if (auto err = convertTreeToVec4Array(rootNode, treeAsVec4))
        {
            SV_ERROR(std::format("Save preset failed: convertTreeToVec4Array failed with {}", *err));
            return false;
        }

        auto packet = Packets::makeTreeAsVec4Packet(treeAsVec4, 0, treeAsVec4.size()-1, presetName.toStdString());
        if (!packet)
        {
            SV_ERROR("Save preset failed: couldnt makePacket from TreeAsVec4Array");
            return false;
        }

        bool saved = writeByteArrayToFile(absPathForPresetVec4File(presetName), *packet);
        if (!saved)
        {
            SV_ERROR("Save preset failed: writing to vec4 file has failed");
            return false;
        }
    }

    // 3) var names data file
    {
        TreeVarNames treeVarNames;
        if (auto err = getVarNamesFromTree(rootNode, treeVarNames))
        {
            SV_ERROR(std::format("Save preset failed: getVarNamesFromTree failed with {}", *err));
            return false;
        }

        auto packet = Packets::makeTreeVarnamesPacket(treeVarNames, presetName.toStdString());
        if (!packet)
        {
            SV_ERROR("Save preset failed: couldnt makePacket from TreeVarNames");
            return false;
        }

        bool saved = writeByteArrayToFile(absPathForPresetVarnamesFile(presetName), *packet);
        if (!saved)
        {
            SV_ERROR("Save preset failed: writing to varnames file has failed");
            return false;
        }
    }


    SV_LOG(std::format("Successfully saved preset {}", presetName));

    return true;
}

bool QTouchApp::saveProject() const
{
    if (!requireProjectIsOpenedFor("Project saving")) return false;

    const auto saveFilePath = getProjectDir()->absoluteFilePath(ProjectJsonFileName);

    QJsonObject json;

    json[PresetExportListKey] = presetTab->getPresetView()->getModel()->savePresetExportListToJson();

    if (!saveJsonValueToFile(json, saveFilePath))
    {
        SV_MSGBOX_ERROR("saveProject: saveJsonValueToFile failed");
        return false;
    }

    return true;
}

StringErrOpt QTouchApp::loadProjectJson(const QString& jsonFilePath)
{
    auto fmtErr = [&](const std::string& text)
    {
        return std::format("loadProjectJson[{}] failed with error: {}", jsonFilePath, text);
    };

    if (!QFile::exists(jsonFilePath))
    {
        //perfectly fine. no data = all default
        return {};
    }

    auto json = loadJsonFromFile(jsonFilePath);
    if (!json)
    {
        return fmtErr("File exists, but couldnt parse json.");
    }

    auto jsonObj = convertJson<QJsonObject>(*json);
    if (!jsonObj)
    {
        return fmtErr("Couldnt convert root to QJsonObject");
    }

    if (!jsonObj->contains(PresetExportListKey))
    {
        return fmtErr(std::format("No key {} !", PresetExportListKey));
    }

    auto presetExportListJson = (*jsonObj)[PresetExportListKey];

    if (!presetTab->getPresetView()->getModel()->loadPresetExportListFromJson(presetExportListJson))
    {
        return fmtErr("preset model couldnt loadPresetExportListFromJson() from what was provided");
    }

    return {};
}

QDirOpt QTouchApp::getProjectDir() const
{
    return projectDir;
}

QDirOpt QTouchApp::getPresetsSubdir() const
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

    if (auto err = loadProjectJson(getProjectDir()->absoluteFilePath(ProjectJsonFileName)))
    {
        SV_MSGBOX_ERROR(*err);
    }

    //presetTab->setEnabled(true);

    centralWidget->setEnabled(true);
    closeProjectAction->setEnabled(true);
    saveProjectAction->setEnabled(true);
    loadCodeAction->setEnabled(true);

    SV_LOG(std::format("Successfully opened project folder [{}]", projectDir->absolutePath()));
    return true;
}

DataNodeShared QTouchApp::getRootNode()
{
    return rootNode;
}

void QTouchApp::deleteExistingTreeAndAllWidgets()
{
    rootNode.reset();
    widgetsView->deleteAllTopLevelWidgets();
    widgetsView->setEnabled(false);

    WidgetsForNodeManager::clear();

    setTreeType(TreeType::Standalone);
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

QString QTouchApp::absPathForPresetJsonFile(const PresetNameString& presetName) const
{
    if (!requireProjectIsOpenedFor("obtaining absolute path for preset files", false))
    {
        return "";
    }

    return getPresetsSubdir()->absoluteFilePath(getPresetJsonFileName(presetName));
}

QString QTouchApp::absPathForPresetVec4File(const PresetNameString& presetName) const
{
    if (!requireProjectIsOpenedFor("obtaining absolute path for preset files", false))
    {
        return "";
    }

    return getPresetsSubdir()->absoluteFilePath(getPresetVec4FileName(presetName));
}

QString QTouchApp::absPathForPresetVarnamesFile(const PresetNameString& presetName) const
{
    if (!requireProjectIsOpenedFor("obtaining absolute path for preset files", false))
    {
        return "";
    }

    return getPresetsSubdir()->absoluteFilePath(getPresetVarnamesFileName(presetName));
}

void QTouchApp::initMenuBar()
{
    auto* projectMenu = menuBar()->addMenu("Project");
    projectMenu->addAction("Create or open existing project", this, &QTouchApp::createOrOpenProjectAction);

    {
        saveProjectAction = new QAction("Save project", this);
        connect(saveProjectAction, &QAction::triggered, this, &QTouchApp::saveProject);

        projectMenu->addAction(saveProjectAction);
    }

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
    
    centralWidget->setDisabled(true);

    closeProjectAction->setDisabled(true);
    saveProjectAction->setDisabled(true);
    loadCodeAction->setDisabled(true);
}

bool QTouchApp::projectIsOpened() const
{
    return projectDir.has_value();
}

bool QTouchApp::requireProjectIsOpenedFor(const char *forOperation, bool withMsgBox) const
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

void QTouchApp::onPresetMixingActivated(const PresetNameString& presetNameA,
                                        const PresetNameString& presetNameB,
                                        double morphAtoB01)
{
    // This is called when:
    //  - preset mixing is turned on
    //  - user moves mix slider to a different position.
    //
    // A lot of the operations here are expensive - trees may be large, constructing
    // or even COMPARING trees is a heavy task! We have to minimize work, whenever possible.
    //
    // For example, if user switched preset mixing on, off, and on again - nothing REALLY changes,
    // so we have to reuse existing trees, not reconstruct them.

    if (!requireProjectIsOpenedFor("Preset mixing")) return;

    //Ensure presets A and B are loaded:
    auto presetDir = getPresetsSubdir();

    auto resultA = presetForMixing_A.loadPresetIfItsNotLoadedYet(presetNameA, absPathForPresetJsonFile(presetNameA));
    auto resultB = presetForMixing_B.loadPresetIfItsNotLoadedYet(presetNameB, absPathForPresetJsonFile(presetNameB));

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
        
        auto treeAndWidgets = createTreeAndWidgetsFromFile(absPathForPresetJsonFile(presetForMixing_B.loadedPresetName));
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

QTouchApp::LoadedPreset::Result QTouchApp::LoadedPreset::loadPresetIfItsNotLoadedYet(const PresetNameString& presetName, const QString& jsonPresetFilePath)
{
    if (rootNode && loadedPresetName == presetName)
    {
        return LoadedPreset::Result::AlreadyHadThisFile;
    }

    if (!QFile::exists(jsonPresetFilePath))
    {
        SV_ERROR(std::format("Load preset from [{}] failed: file doesnt exist", jsonPresetFilePath));

        clear();
        return LoadedPreset::Result::Error;
    }

    auto json = loadJsonFromFile(jsonPresetFilePath);
    if (!json)
    {
        SV_ERROR(std::format("Load preset from [{}] failed: couldnt parse json from file", jsonPresetFilePath));

        clear();
        return LoadedPreset::Result::Error;
    }

    rootNode = DataNode::fromJSON(json.value());
    if (!rootNode)
    {
        SV_ERROR(std::format("Load preset from [{}] failed: couldnt parse data tree", jsonPresetFilePath));

        clear();
        return LoadedPreset::Result::Error;
    }

    //Successfully loaded then.
    loadedPresetName = presetName;
    return LoadedPreset::Result::JustLoadedThisFile;
}

void QTouchApp::LoadedPreset::clear()
{
    rootNode.reset();
    loadedPresetName.clear();
}

void QTouchApp::exportPresets()
{
    if (auto err = tryExportPresets())
    {
        SV_MSGBOX_ERROR(std::format("Couldnt send export presets to Touchdesigner due to error:\n{}", *err));
    }
}

StringErrOpt QTouchApp::tryExportPresets()
{
    if (!requireProjectIsOpenedFor("Presets export")) return "Project is not opened";

    //even if there are no presets to export, we must still send packet with zero exports,
    //so user can clear exports in Touchdesigner this way.

    const auto& exportPresetNames = presetTab->getPresetView()->getModel()->getPresetExportList();

    std::vector<QByteArray> vec4Packets;

    QByteArrayOpt firstVarnamesData;
    QString firstPresetName;

    for (const auto& presetName : exportPresetNames)
    {
        auto vec4File       = absPathForPresetVec4File      (presetName);
        auto varnamesFile   = absPathForPresetVarnamesFile  (presetName);

        auto vec4Data = readByteArrayFromFile(vec4File);
        if (!vec4Data)
        {
            return std::format("error reading file [{}]", vec4File);
        }
        vec4Packets.push_back(std::move(*vec4Data));

        auto varnamesData = readByteArrayFromFile(varnamesFile);
        if (!varnamesData)
        {
            return std::format("error reading file [{}]", varnamesFile);
        }

        if (!firstVarnamesData)
        {
            firstVarnamesData = varnamesData;
            firstPresetName = presetName;
        }
        else //current data is not first, so we should compare it against first:
        {
            if (!Packets::varnamesPacketsContentIsSame(*varnamesData, *firstVarnamesData))
            {
                return std::format( "Mismatch in varnames content:\n"
                                    "Preset [{}] packet size [{}]\n"
                                    "Preset [{}] packet size [{}]\n",
                                    firstPresetName, firstVarnamesData->size(),
                                    presetName, varnamesData->size() );
            }
        }
    }

    auto packet = Packets::makePresetExportsPacket(firstVarnamesData.value_or(QByteArray()), vec4Packets);
    if (!packet)
    {
        return "couldnt create preset export packet";
    }

    tdClient->sendPacket(*packet);

    return {};
}