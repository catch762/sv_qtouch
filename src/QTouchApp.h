#pragma once
#include "sv_qtcommon.h"
#include <QMainWindow>

#include "SUP_Data/SUP_DataParser.h"
#include "TreeAndWidgetsBuilder.h"

#include "TouchdesignerCommunication/TDTcpCLient.h"

#include "QTouchDefs.h"

class PresetTab;
class TopLevelWidgetsContainer;
class QTouchApp : public QMainWindow
{
public:
    QTouchApp(QWidget *parent = nullptr);

    bool loadTreeAndWidgetsFromCode             (const QStringVec& codeFilePaths);
    bool loadTreeAndWidgetsFromPresetFile       (const QString& filePath);
    bool loadTreeAndWidgetsUsingPresetFileName  (const PresetNameString& presetName);

    //returns success
    bool openProjectDir(const QDir& newProjectDir);

    DataNodeShared getRootNode();

    //returns success
    bool savePreset(const PresetNameString& presetName) const;

    bool saveProject() const;

    //**** 1) Checking if project is opened ****
    bool projectIsOpened() const;
    //**** 2) So, if project IS opened, these dirs will have value, otherwise they wont ****
    QDirOpt getProjectDir() const;
    QDirOpt    getPresetsSubdir() const;

private slots:
    void onPresetMixingActivated(const PresetNameString& presetNameA, const PresetNameString& presetNameB, double morphAtoB01);
    
private:
    struct LoadedPreset
    {
    public:
        DataNodeShared rootNode;
        PresetNameString loadedPresetName;

    public:
        enum Result
        {
            Error,
            JustLoadedThisFile,
            AlreadyHadThisFile
        };
        Result loadPresetIfItsNotLoadedYet(const PresetNameString& presetName, const QString& jsonPresetFilePath);
        void clear();
    };

    enum TreeType
    {
        Standalone,
        IsMixResult
    };
    void setTreeType(TreeType type);

    void deleteExistingTreeAndAllWidgets();
    
    static std::optional< std::tuple<DataNodeShared, NodeWidgetVec> > createTreeAndWidgetsFromFile(const QString& filePath);

    
    QString absPathForPresetJsonFile(const PresetNameString& presetName) const;
    

    //returns same as 'projectIsOpened()' and if its not, prints error
    bool requireProjectIsOpenedFor(const char* forOperation, bool withMsgBox = true) const;

    StringErrOpt loadProjectJson(const QString& jsonFilePath);

//Menu bar and its actions:
private:
    void initMenuBar();
    void createOrOpenProjectAction();
    void closeProject();

    QAction* closeProjectAction = nullptr;
    QAction* loadCodeAction = nullptr;
    QAction* saveProjectAction = nullptr;

private:
    QDirOpt projectDir; //no value means no project is opened

    TreeType rootType = TreeType::Standalone; //Standalone  -> rootNode is data tree on its own, standard mode
                                              //IsMixResult -> rootNode is mix of preset A and B
    DataNodeShared  rootNode;  //invisible root, no widget is created for this root node
                               //widgets for all immediate children of 'rootNode' are stored in 'widgetsView'

    //When we switch off mixing mode - we just leave them be, so we dont
    //have to load same files later, if we need to.
    LoadedPreset presetForMixing_A;
    LoadedPreset presetForMixing_B;

    TDTcpClient* tdClient = nullptr;

private:
    QWidget*                    centralWidget   = nullptr;
    QHBoxLayout*                centralLayout   = nullptr;
    TopLevelWidgetsContainer*       widgetsView = nullptr;
    PresetTab*                  presetTab       = nullptr;
};