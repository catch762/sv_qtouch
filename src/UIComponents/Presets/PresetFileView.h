#pragma once
#include "sv_qtcommon.h"
#include <QTableView>
#include <QMenu>
#include <QMessageBox>
#include <QInputDialog>
#include "PresetFileSystemModel.h"
#include "QTouchDefs.h"

class PresetFileView : public QWidget
{
    Q_OBJECT
public:
    PresetFileView(const QString& rootPath, QWidget* parent = nullptr);

    bool presetNameExists(const PresetNameString& presetName);

    void setRootPath(const QString& rootPath);

    void onContextMenu(const QPoint &pos);

    PresetFileSystemModel* getModel();

signals:
    //selected for preset A or for preset B
    void presetWasSelectedForMixing (const PresetNameString& presetName, bool selectedForA);
    void presetLoadingRequested     (const PresetNameString& presetName);

private:
    void deleteSelectedFiles(const QModelIndexList& selectedRows);

private:
    QVBoxLayout* layout = nullptr;

    QTableView       *view = nullptr;
    PresetFileSystemModel* model = nullptr;

};